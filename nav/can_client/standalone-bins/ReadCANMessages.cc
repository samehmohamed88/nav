
#include "nav/can_client/CANDBC.h"
#include "nav/can_client/UsbDevice.h"
#include "nav/can_client/LibUsbDevice.h"
#include "nav/can_client/CommaAICANInterfaceWithSimplyDeque.h"

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <libusb-1.0/libusb.h>

#include <fstream>
#include <vector>
#include <iostream>
#include <cstdint>  // For uint8_t

using namespace nav;

void perform_usb_transfer(const boost::system::error_code& /*e*/, boost::asio::steady_timer* t, int* count, can::CommaAICANInterfaceWithSimplyDeque<can::UsbDevice<can::LibUsbDevice>>* canDevice) {
    if (*count < 100) {
        std::cout << "USB Transfer " << *count << std::endl;
        ++(*count);

        // Example USB bulk transfer parameters
//        unsigned char endpoint = 0x81; // Example endpoint
//        unsigned char data[512];       // Buffer for data
//        int actual_length;             // Will store the actual length of data received
//        int timeout = 1000;            // Timeout in ms
//
//        // Perform the bulk transfer
//        int transfer_result = libusb_bulk_transfer(handle, endpoint, data, sizeof(data), &actual_length, timeout);
//
//        if (transfer_result == 0) {
//            // Handle successful transfer
//        } else {
//            // Handle errors
//        }

//        (*canDevice).getCANMessages();
        // Schedule the next transfer



        t->expires_at(t->expiry() + boost::asio::chrono::milliseconds(10));
        t->async_wait(boost::bind(perform_usb_transfer, boost::asio::placeholders::error, t, count, canDevice));
    }
}

int main() {
//    boost::asio::io_context io;
//    int count = 0;
//    boost::asio::steady_timer t(io, boost::asio::chrono::milliseconds(10));
//    t.async_wait(boost::bind(perform_usb_transfer, boost::asio::placeholders::error, &t, &count, &canDevice));
//    io.run();

    using namespace nav;
    static constexpr uint16_t vendorID_ = 0xbbaa;
    static constexpr uint16_t productID_ = 0xddcc;

    auto device = std::make_unique<can::UsbDevice<can::LibUsbDevice>>(
            std::make_unique<can::LibUsbDevice>(),
                    vendorID_,
                    productID_,
                    0);

    auto canDevice = can::CommaAICANInterfaceWithSimplyDeque<can::UsbDevice<can::LibUsbDevice>>{std::move(device)};
    auto hw = canDevice.getHardwareType();
    assert(hw == 7);
    std::cout << "Hardware Type == " << std::to_string(hw) << std::endl;

    std::string filename = "/apollo/can_uint8_t.txt"; // Replace with your filename
    std::ifstream file(filename, std::ios::binary);
    const size_t chunkSize = 0x1000U; // Size of each chunk in bytes
    std::vector<uint8_t> buffer(chunkSize);


    auto thread1 = std::thread([&]{
        while (file) {
            auto messages = canDevice.getCANMessagesAndClearContainer();
//            std::cout << " size of messages " << messages.size() << std::endl;
//            for (auto const& message : messages) {
//                std::cout << "Message name is "
//                          << message.name
//                          << " and address is "
//                          << message.address
//                          << " and signal size "
//                          << message.signals.size() << std::endl;
//            }
        }
    });

    auto thread2 = std::thread([&]{
        while (file) {
            file.read(reinterpret_cast<char*>(buffer.data()), chunkSize);
            size_t bytesRead = file.gcount();
            std::cout << " number of bytes read = " << bytesRead << std::endl;
            if (bytesRead > 0) {
                buffer.resize(bytesRead); // Resize buffer to actual bytes read
                canDevice.receiveMessages(buffer);
                buffer.resize(chunkSize); // Resize back for next read
            }
        }
    });
    thread1.join();
    thread2.join();

    return 0;
}
