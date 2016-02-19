//
// Created by sergey on 19.02.16.
//

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <QObject>

#include "../HDM50.h"

class PacketGenerator: public ::testing::Test {
protected:
    //virtual void SetUp() {}
    HDM50Protocol protocol;
};

TEST_F(PacketGenerator, CommandGeneratorTest) {
    ASSERT_THAT(
            protocol.cmd_read_prh(),
            ::testing::ElementsAre(0x68, 0x04, 0x00, 0x04, 0x08)) << "PRH command generator error";
    ASSERT_THAT(
            protocol.cmd_start_calibrate(),
            ::testing::ElementsAre(0x68, 0x04, 0x00, 0x08, 0x0F)) << "Calibrate command generator error";
}
