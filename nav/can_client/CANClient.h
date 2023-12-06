#pragma once

#include "nav/can_client/CANDBC.h"

#include "cyber/common/log.h"

#include <units.h>
#include <boost/circular_buffer.hpp>

#include <chrono>
#include <vector>
#include <cmath>

namespace nav {
namespace can {

template <class CANDevice>
class CANClient {
public:
    CANClient(std::unique_ptr<CANDevice> canDevice, std::unique_ptr<CANDBC> canDatabase);

    /// Waits for CAN messages to appear
    bool waitForMessages(units::time::millisecond_t timeout = units::time::millisecond_t(3000));

    /// Begin reading and buffer messages from the CAN Device
    template <class CANDeviceMessage>
    void getMessages();

    /// Attempts to send a single CAN message
    bool sendMessage(const CANDBCMessage &message);

    /// Attemps to send multiple CAN messages;
    bool sendMessages(const std::vector<CANDBCMessage> &messages);

    /// Returns all queued messages and clears the Queue
    std::vector<CANDBCMessage> getQueuedMessagesAndClearQueue();
private:
    /// Attempts to read a single message from the bus
    template <class CANDeviceMessage>
    CANDeviceMessage getMessage();
    void queueMessage(const CANDBCMessage& message);
private:
    mutable std::mutex qMutex_;
    std::unique_ptr<CANDevice> canDevice_;
    std::unique_ptr<CANDBC> canDatabase_;
    std::vector<CANDBCMessage> canMessages_;
    std::unordered_map<uint32_t, uint32_t> messageCountsByAddress_;
    /// TODO: we want this in an external config.
    static constexpr bool ignoreChecksum = true;
    /// TODO: we want this in an external config.
    static constexpr bool ignoreCounter = false;
};

template <class CANDevice>
CANClient<CANDevice>::CANClient(std::unique_ptr<CANDevice> canDevice, std::unique_ptr<CANDBC> canDatabase) :
    qMutex_{}
    , canDevice_{std::move(canDevice)}
    , canDatabase_{std::move(canDatabase)}
    , canMessages_{}
{

}

template <class CANDevice>
template <class CANDeviceMessage>
CANDeviceMessage CANClient<CANDevice>::getMessage() {
    return canDevice_->getMessage();
}

template <class CANDevice>
template <class CANDeviceMessage>
void CANClient<CANDevice>::getMessages() {
    CANDeviceMessage canDeviceMessage = canDevice_->getMessage();
    if (canDeviceMessage.isValid()) {
        CANDBCMessage message;
        CANDBCMessage canMessage;
        canMessage.address = canDeviceMessage.address;
//        canMessage.canBus = canDeviceMessage.canBus;

        auto const &signalsRef = canDatabase_->getSignalSchemasByAddress(canDeviceMessage.address);
        if (signalsRef.has_value()) {
            auto const &signals = signalsRef.value().get();
            canMessage.signals.reserve(signals.size());
            canMessage.name = signals[0].messageName;

            for (auto const &signalSchema: signals) {
                int64_t tmp = signalSchema.parseValue(canDeviceMessage.getFrameData());

                if (signalSchema.is_signed) {
                    tmp -= ((tmp >> (signalSchema.size - 1)) & 0x1) ? (1ULL << signalSchema.size) : 0;
                }
                bool checksum_failed = false;
                if (!ignoreChecksum) {
                    if (signalSchema.calcSubaruChecksum(canDeviceMessage.address, canDeviceMessage.getFrameData()) != tmp) {
                        checksum_failed = true;
                    }
                }
                bool counter_failed = false;
//                if (!ignoreCounter) {
//                    if (signalSchema.type == SignalType::COUNTER) {
//                        counter_failed = !updateCounter(dataFrame, tmp, signalSchema.size);
//                    }
//                }
                if (checksum_failed || counter_failed) {
                    AERROR << "Message checks failed: "
                           << canDeviceMessage.address
                           << " checksum failed " << checksum_failed
                           << " counter failed " << counter_failed;
                    continue;
                }
                auto &parsedSignal = canMessage.signals.emplace_back();
                parsedSignal.value = tmp * signalSchema.factor + signalSchema.offset;
                parsedSignal.name = signalSchema.name;
            }
            // we add the can message to the CAN Message Queue which is picked up by the CAN Timer Component
            // on a specified interval, converted to DDS messages and cleared.
            queueMessage(std::move(canMessage));
        }
    }
}

template <class CANDevice>
std::vector<CANDBCMessage> CANClient<CANDevice>::getQueuedMessagesAndClearQueue() {
    std::lock_guard<std::mutex> lock(qMutex_);
    // Move the entire vector
    std::vector<CANDBCMessage> currentMessages = std::move(canMessages_);
    // After the move, canMessages is empty
    // No need to call clear()
    return currentMessages;
}

template <class CANDevice>
void CANClient<CANDevice>::queueMessage(const CANDBCMessage& message) {
    std::lock_guard<std::mutex> lock(qMutex_);
    canMessages_.push_back(message);
}

template <class CANDevice>
bool CANClient<CANDevice>:: sendMessage(const CANDBCMessage &message) {
    // set all values for all given signal/value pairs
    bool setCounterSignal = false;
    const auto messageSchemaRef = canDatabase_->getMessageByName(message.name);

    // we lookup message schema by name and make sure we don't get a nullptr
    if (messageSchemaRef.has_value()) {
        // get the MessageSchema from the std::optional and begin processing message
        const auto& messageSchema = messageSchemaRef.value().get();

        // first we make a std::vector to hold the raw data that will be transferred over the CAN bus
        std::vector<uint8_t> rawData(messageSchema.size, 0);

        for (const auto& messageSignal : message.signals) {
            // we lookup the Signal Schema by name inside the MessageSchema and make sure we don't get a nullptr
            const auto signalSchemaRef = messageSchema.getSignalSchmeByName(messageSignal.name);
            if (signalSchemaRef.has_value()) {
                // get the Signal Schema from std::optional
                const auto signalSchema = signalSchemaRef.value().get();

                // convert value to raw value using signal schema offset and factor
                int64_t tmp_val = (int64_t)(std::round((messageSignal.value - signalSchema.offset) / signalSchema.factor));
                if (tmp_val < 0) {
                    tmp_val = (1ULL << signalSchema.size) + tmp_val;
                }
                signalSchema.packValue(rawData, tmp_val);

                setCounterSignal = setCounterSignal || (signalSchema.name == "COUNTER");
                if (setCounterSignal) {
                    messageCountsByAddress_[message.address] = messageSignal.value;
                }
            }
        }

        // set message counter
        // we lookup the COUNT SignalSchema by name inside the MessageSchema and make sure we don't get a nullptr
        const auto countSignalSchemaRef = messageSchema.getSignalSchmeByName("COUNT");
        // setCounterSignal false means that the CANMessage above did not supply a COUNT Signal
        // for example ES_LKAS message does not supply COUNT in code so we set it here
        if (!setCounterSignal && countSignalSchemaRef.has_value()) {
            const auto& countSignalSchema = countSignalSchemaRef.value().get();

            // we zero out the counter first if it exists
            if (messageCountsByAddress_.find(message.address) == messageCountsByAddress_.end()) {
                messageCountsByAddress_[message.address] = 0;
            }
            countSignalSchema.packValue(rawData, messageCountsByAddress_[message.address]);
            messageCountsByAddress_[message.address] = (messageCountsByAddress_[message.address] + 1) % (1 << countSignalSchema.size);
        }

        // set message checksum
        // we lookup the CHECKSUM SignalSchema by name inside the MessageSchema and make sure we don't get a nullptr
        const auto checksumSignalSchemaRef = messageSchema.getSignalSchmeByName("CHECKSUM");
        if (checksumSignalSchemaRef.has_value()) {
            const auto& checksumSignalSchema = checksumSignalSchemaRef.value().get();

            // use the Subaru checksum calculation copies from Comma AI
            unsigned int checksum = checksumSignalSchema.calcSubaruChecksum(message.address, rawData);

            // pack the checksum into the output buffer
            checksumSignalSchema.packValue(rawData, checksum);
        }

        uint8_t dataLengthCode = CANDBC::bufferSizeToDataLengthCode(rawData.size());
        assert(rawData.size() <= 64);
        // assert we did the converstion back successfully
        assert(rawData.size() == CANDBC::dataLengthCodeToNumBytes[dataLengthCode]);

        CANHeader canHeader{};
        canHeader.addr = message.address;
        canHeader.extended = (message.address >= 0x800) ? 1 : 0;
        canHeader.dataLengthCode = dataLengthCode;
        canHeader.bus = static_cast<uint8_t>(message.canBus);
        canHeader.checksum = 0;

        memcpy(&canBuffer[bufferLength], &canHeader, sizeof(CANHeader));
        memcpy(&canBuffer[bufferLength + sizeof(CANHeader)], rawData.data(), rawData.size());
        uint32_t msg_size = sizeof(CANHeader) + rawData.size();

        // set checksum
        ((CANHeader *) &canBuffer[bufferLength])->checksum = calculate_checksum(&canBuffer[bufferLength], msg_size);

        bufferLength += msg_size;
        if (bufferLength >= USB_TX_SOFT_LIMIT) {
            std::vector<uint8_t> transferData;
            transferData.insert(
                    transferData.end(),
                    std::make_move_iterator(canBuffer.begin()),
                    std::make_move_iterator(canBuffer.begin() + bufferLength)
            );
            device_->bulkWrite(
                    static_cast<uint8_t>(DeviceRequests::WRITE_TO_CAN_BUS),
                    transferData,
                    std::make_unique<int>(0));
            bufferLength = 0;
            return transferData;
        }
}

} // namespace can
} // namespace nav

