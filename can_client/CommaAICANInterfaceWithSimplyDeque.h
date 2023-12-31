#pragma once

#include "can_client/CANDBC.h"
#include "can_client/DeviceInfo.h"
#include "cyber/common/log.h"

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace nav {
namespace can {

template <class Device>
class CommaAICANInterfaceWithSimplyDeque {
public:
    /// the Comma AI Red Panda Vendor ID
    static constexpr uint16_t vendorID_ = 0xbbaa;
    /// thee Comma AI Red Panda Product ID
    static constexpr uint16_t productID_ = 0xddcc;
    /// interface number taken from https://github.com/commaai/openpilot/blob/master/selfdrive/boardd/panda_comms.cc#L73
    static constexpr int interfaceNumber_ = 0;
public:
    CommaAICANInterfaceWithSimplyDeque(std::unique_ptr<Device> device);

    /// Sets the safety model on the CAN Device
    /// The Comma AI CAN Device has a specific safety model where it starts in silent mode with No Output to prevent
    /// accidental movements/steering
    /// taken from https://github.com/commaai/cereal/blob/416c3d531c90ce16498d782bf383625a857ee74c/car.capnp#L567C8-L567C19
    bool setSafetyModel(SafetyModel safety_model, uint16_t safety_param=0U);

    uint8_t getHardwareType();
    std::vector<CANMessage> getCANMessagesAndClearContainer() {
        std::lock_guard<std::mutex> lock(mtx);
        // Move the entire vector
        std::vector<CANMessage> currentMessages = std::move(canMessages);

        // After the move, canMessages is empty
        // No need to call clear()

        return currentMessages;
    }
    bool receiveMessages(std::vector<uint8_t>& chunk);

private:
    void addCANMessage(const CANMessage& message);
    bool parseRawCANToCANFrame();
    bool parseCANFrameToCANMessage();
    void pushCANDataAndPopFromRawBuffer(const CANHeader &canHeader,
                                        size_t dataFrameLength);

    uint8_t calculate_checksum(const uint8_t *data, uint32_t len) {
        uint8_t checksum = 0U;
        for (uint32_t i = 0U; i < len; i++) {
            checksum ^= data[i];
        }
        return checksum;
    }

    int64_t parseValueUsingSignalSchema(const std::vector<uint8_t> &messageData, const CANDBC::SignalSchema signalSchema) {
        int64_t ret = 0;
        int i = signalSchema.mostSignificantBit / 8;
        int bits = signalSchema.size;
        while (i >= 0 && i < static_cast<int>(messageData.size()) && bits > 0) {
            int lsb = (int)(signalSchema.leastSignificantBit / 8) == i ? signalSchema.leastSignificantBit : i*8;
            int msb = (int)(signalSchema.mostSignificantBit / 8) == i ? signalSchema.mostSignificantBit : (i+1)*8 - 1;
            int size = msb - lsb + 1;

            uint64_t d = (messageData[i] >> (lsb - (i*8))) & ((1ULL << size) - 1);
            ret |= d << (bits - size);

            bits -= size;
            i = signalSchema.isLittleEndian ? i-1 : i+1;
        }
        return ret;
    }

private:
    mutable std::mutex mtx;
    std::unique_ptr<Device> device_;
    std::deque<uint8_t> receiveBuffer_;
    std::vector<CANFrame> canDataFrames_;
    std::vector<CANMessage> canMessages;
    std::unique_ptr<CANDBC> canDatabase_;
    std::unique_ptr<std::vector<uint8_t>> temp_buffer;


    static constexpr size_t MAX_BUFFER_SIZE = 0x4000U; // Max buffer size
    static constexpr auto CAN_REJECTED_BUS_OFFSET = 0xC0U;
    static constexpr auto CAN_RETURNED_BUS_OFFSET = 0x80U;
};

template <class T>
bool CommaAICANInterfaceWithSimplyDeque<T>::setSafetyModel(SafetyModel safetyModel, uint16_t safetyParam) {
    DeviceStatus status = device_.controlWrite(DeviceRequests::SafetyModel, static_cast<uint16_t>(safetyModel), safetyParam);
    return status == DeviceStatus::SUCCESS;
}

template <class Device>
CommaAICANInterfaceWithSimplyDeque<Device>::CommaAICANInterfaceWithSimplyDeque(std::unique_ptr<Device> device) :
        mtx{}
        , device_{std::move(device)}
        , receiveBuffer_{}
        , canDataFrames_{}
        , canMessages{}
        , canDatabase_{CANDBC::CreateInstance()}
        , temp_buffer{std::make_unique<std::vector<uint8_t>>()}
        {
            temp_buffer->reserve(MAX_BUFFER_SIZE + sizeof(CANHeader) + 64);
            for (size_t i =0 ; i < (MAX_BUFFER_SIZE + sizeof(CANHeader) + 64); i++){
                temp_buffer->push_back(0);
//        std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>.";
            }
        }

template <class Device>
uint8_t CommaAICANInterfaceWithSimplyDeque<Device>::getHardwareType() {
    std::vector<uint8_t> data{0};
    // This is used to pass a parameter to the device, specific to the request, in this case 0
    uint16_t requestValue = 0;
    // The index field for the setup packet, this is often 0
    uint16_t requestIndex = 0;
    DeviceStatus status = device_->controlRead(
            Device::ReadRequest,
            static_cast<uint8_t>(DeviceRequests::HardwareType),
            requestValue,
            requestIndex,
            data);
    if (status == DeviceStatus::SUCCESS) {
        return data[0];
    }
    return 200;
}

//template <class Device>
//std::vector<CANMessage> CommaAICANInterfaceWithSimplyDeque<Device>::getCANMessagesAndClearContainer() {
//
//}

template <class Device>
bool CommaAICANInterfaceWithSimplyDeque<Device>::receiveMessages(std::vector<uint8_t>& chunk) {
//    std::shared_ptr<int> transferred = std::make_shared<int>(0);
//    device_->bulkRead(
//            static_cast<uint8_t>(DeviceRequests::READ_CAN_BUS),
//            *temp_buffer.get(),
//            transferred);


    // this actual number of bytes transferred from USB
    // let's dereference once so we can reuse it in multiple checks
//    int received = *transferred;

//    auto temp_buffer = chunk.data();
    int received = chunk.size();

    //let's add everything received onto the buffer
    for (int i = 0; i < received; i++) {
        receiveBuffer_.push_back(chunk[i]);
    }

    // the front of the buffer should always be a can header
//    CANHeader header;
//    memcpy(&header, receiveBuffer_.be, sizeof(CANHeader));

//    if (!device_.isCommHealthy()) {
//        return false;
//    }
//    if (received == MAX_BUFFER_SIZE) {
//        AWARN << " The Panda Receive Buffer is Full";
//    }
    // Check if adding new data exceeds max buffer size
//    if (receiveBuffer_.size() + received > MAX_BUFFER_SIZE) {
//        // Handle buffer overflow, e.g., log error, discard data, etc.
//        AWARN << "Exceeding maximum buffer size, discarding data";
//        return false;
//    }
    // Add received bytes to the deque
//    receiveBuffer_.insert(receiveBuffer_.end(), received, *temp_buffer->data());

//    std::move(begin(*temp_buffer.get()), received, std::back_inserter(receiveBuffer_));
//    for(int i = 0; i < received; i++){
////        receiveBuffer_.push_front((*temp_buffer)[i]);
//        receiveBuffer_.push_back(chunk[i]);
//    }
//    bool invalid = (received <= 0);
//    invalid &= parseRawCANToCANFrame();

    // we clean the vector of CAN Data Frames before processing more
//    canDataFrames_.clear();

//    parseRawCANToCANFrame();
//    parseCANFrameToCANMessage();
//    if (valid) {

//    } else {
//        for(int i = 0; i < received; i++){
//        receiveBuffer_.push_front((*temp_buffer)[i]);
//            receiveBuffer_.pop_front();
//        }
//    }


//    return received <= 0 ? true : unpack_can_buffer(receiveBuffer_);
    return true;
}

template <class Device>
void CommaAICANInterfaceWithSimplyDeque<Device>::addCANMessage(const CANMessage& message) {
    std::lock_guard<std::mutex> lock(mtx);
    canMessages.push_back(message);
}

template <class Device>
void CommaAICANInterfaceWithSimplyDeque<Device>::pushCANDataAndPopFromRawBuffer(const CANHeader &canHeader,
                                                                                size_t dataFrameLength) {
    uint32_t bus_offset = 0;
    CANFrame &canFrame = canDataFrames_.emplace_back();
    // Set canData properties
    canFrame.busTime = 0;
    canFrame.address = canHeader.addr;
    canFrame.src = canHeader.bus + bus_offset;
    if (canHeader.rejected) {
        canFrame.src += CAN_REJECTED_BUS_OFFSET;
    }
    if (canHeader.returned) {
        canFrame.src += CAN_RETURNED_BUS_OFFSET;
    }
//        if (calculate_checksum(receiveBuffer_.begin(), sizeof(CANHeader) + dataLength) != 0) {
//            AERROR << "Panda CAN checksum failed";
//            return false;
//        }

    // we can now pop the entire contents of the CAN Header from the buffer
//    receiveBuffer_.erase(receiveBuffer_.begin(), receiveBuffer_.begin() + sizeof(CANHeader));
    for (size_t i = 0; i < sizeof(CANHeader); i++) {
        receiveBuffer_.pop_front();
    }
    // we move and pop the CAN frame data from the buffer to the CANFrame structure
    canFrame.data.reserve(dataFrameLength);
    for (size_t i = 0; i < dataFrameLength; ++i) {
        canFrame.data.push_back(std::move(receiveBuffer_.front()));
        receiveBuffer_.pop_front();
    }
}

template <class Device>
bool CommaAICANInterfaceWithSimplyDeque<Device>::parseRawCANToCANFrame() {
    size_t position = 0;
    while (receiveBuffer_.size() >= (position + sizeof(CANHeader))) {
        CANHeader header;
        auto X = sizeof(CANHeader);
        // we copy to a temp vector to prevent potential errors in  memory layout introduced by manual copying directly
        // into the receivedBuffer std::deque
        std::vector<uint8_t> tempBuffer(receiveBuffer_.begin() + position, receiveBuffer_.begin() + position + sizeof(CANHeader));
        std::memcpy(&header, tempBuffer.data(), sizeof(CANHeader));
        std::cout << "data len code " << std::to_string(header.data_len_code) << X << " " << std::endl;;
        if (header.data_len_code == 0) {
//            throw std::runtime_error(std::to_string(header.data_len_code));
                return false;
        }
        const uint8_t dataLength = CANDBC::dataLengthCodeToNumBytes[header.data_len_code];

        if (receiveBuffer_.size() < sizeof(CANHeader) + dataLength) {
            // we don't have all the data for this message yet
            // so we leave the data on the buffer and the next iteration of receiveMessages() should
            // append the remainder of the message to the buffer
            break;
        }
        // we have a full message so we parse the raw buffer to a CANFrame struct
        // using the information from the header to add the CANFrame data to the canDataFrames_ std::vector
        pushCANDataAndPopFromRawBuffer(header, dataLength);

        // we increase the position and move the overflowing data to the beginning of the buffer
        // for the next iteration of the while loop
        position += sizeof(CANHeader) + dataLength;

    }
    return true;
}

template <class Device>
bool CommaAICANInterfaceWithSimplyDeque<Device>::parseCANFrameToCANMessage() {
    for (auto const& dataFrame : canDataFrames_) {
        CANMessage canMessage;
        canMessage.address = dataFrame.address;

        auto const& signalsRef = canDatabase_->getSignalSchemasByAddress(dataFrame.address);
        if (signalsRef.has_value()) {

            auto const& signals = signalsRef.value().get();
            canMessage.signals.reserve(signals.size());
            canMessage.name = signals[0].messageName;


            for (auto const& signalSchema : signals) {
                auto &parsedSignal = canMessage.signals.emplace_back();
                int64_t tmp = parseValueUsingSignalSchema(dataFrame.data, signalSchema);
                if (signalSchema.is_signed) {
                    tmp -= ((tmp >> (signalSchema.size-1)) & 0x1) ? (1ULL << signalSchema.size) : 0;
                }

//                AINFO << "parse 0x%X %s -> %ld\n" << address <<  sig.name, tmp);
//                bool checksum_failed = false;
//                if (!ignore_checksum) {
//                    if (sig.calc_checksum != nullptr && sig.calc_checksum(address, sig, dat) != tmp) {
//                        checksum_failed = true;
//                    }
//                }
//                bool counter_failed = false;
//                if (!ignore_counter) {
//                    if (sig.type == SignalType::COUNTER) {
//                        counter_failed = !update_counter_generic(tmp, sig.size);
//                    }
//                }
//                if (checksum_failed || counter_failed) {
//                    LOGE("0x%X message checks failed, checksum failed %d, counter failed %d", address, checksum_failed, counter_failed);
//                    return false;
//                }
                parsedSignal.value = tmp * signalSchema.factor + signalSchema.offset;
                parsedSignal.name = signalSchema.name;
                if (canMessage.name == "Steering_Torque") {
//                throw std::runtime_error("FOUND");
                    if (parsedSignal.name == "Steering_Torque" && parsedSignal.value > 0) {
                        std::cout << ">>>>>>>>>>>>> HAPPY " << parsedSignal.value;
                    }
                }
//                all_vals[i].push_back(vals[i]);
            }
        }
        // we add the can message to the CAN Message Queue which is picked up by the CAN Timer Component
        // on a specified interval, converted to DDS messages and cleared.
        addCANMessage(std::move(canMessage));
    }
    return true;
}

//template <class Device>
//bool CommaAICANInterface<Device>::receiveMessages(const boost::system::error_code& /*e*/, boost::asio::steady_timer* t, int* count) {
//    // holds the value of the actual number of bytes read
//    std::shared_ptr<int> transferred = std::make_shared<int>(0);
//        DeviceStatus status = device_->bulkRead(
//            static_cast<uint8_t>(DeviceRequests::READ_CAN_BUS),
//            dataBuffer_,
//            transferred);

//    constexpr auto vectorSize = 0x4000U;
//
//    std::vector<uint8_t> data;
//    data.reserve(vectorSize);
//    for (unsigned int i =0; i < vectorSize; i++) {
//        data.push_back(0);
//    }
//    std::cout << "We started with " << std::to_string(data[0]) << std::endl;

//    std::cout << "Panda returned " << std::to_string(data[0]) << std::endl;
////    std::ofstream oufile("/home/sameh/can_example.txt"+std::to_string(*transferred));
////    oufile.open();
//    std::ostream_iterator<std::uint8_t> output_iterator(output_file_, "\n");
//    std::copy(std::begin(data), std::end(data), output_iterator);
////    for (int i = 0; i  < *transferred; i++) {
////        oufile << data[i];
////    }
////    oufile << "\n";
//    if (status == DeviceStatus::SUCCESS) {
//        return true;
//    }
//    return false;
//}

} // namespace can
} // namespace nav