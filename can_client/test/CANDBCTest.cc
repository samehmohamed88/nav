
#include "can_client/CANDBC.h"

#include <gtest/gtest.h>

namespace nav {
namespace tests {

TEST(CANDBCTest, TestCanDbcConstructor) {
    auto device = nav::can::CANDBC::CreateInstance();
    // should return the steering message
    auto message = device->getMessageByAddress(2);
    EXPECT_EQ(message.value().get().getName(), "Steering");
    EXPECT_EQ(message.value().get().getSignals().size(), 3);
}

}
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}