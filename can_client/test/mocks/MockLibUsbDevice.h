#include "can_client/LibUsbDeviceInterface.h"

#include <gmock/gmock.h>
#include <libusb-1.0/libusb.h>
#include <units.h>

namespace nav {
namespace mocks {
class MockLibUsbDevice : public can::LibUsbDeviceInterface {
public:
    MOCK_METHOD(int, libUsbControlTransfer, (
            uint8_t bmRequestType,
            uint8_t bRequest,
            uint16_t wValue,
            uint16_t wIndex,
            unsigned char *data,
            uint16_t wLength,
            unsigned int timeout), (const override));

    MOCK_METHOD(int, libUsbBulkTransfer, (
            unsigned char endpoint,
            unsigned char *data,
            int length,
            int *actual_length,
            unsigned int timeout), (const override));

    MOCK_METHOD(libusb_error, libUsbInitDevice, (), (override));

    MOCK_METHOD(libusb_error, libUsbOpenDevice, (uint16_t vendorID_, uint16_t productID_), (override));

    MOCK_METHOD(void, libUsbReleaseDevice, (), (override));

    MOCK_METHOD(libusb_error, libUsbSetDefaultConfiguration, (), (const override));

    MOCK_METHOD(libusb_error, libUsbDetachKernelDriver, (int interfaceNumber), (const override));

    MOCK_METHOD(libusb_error, libUsbClaimInterface, (int interfaceNumber), (const override));

};
}
}