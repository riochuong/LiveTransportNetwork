#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>

#include "network-monitor/stomp-frame.h"

using namespace NetworkMonitor;

TEST(TestStompFrameParsing, TestParseWellFormed){

    std::string frame =
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body\0";

    StompError ec;
    StompFrame(ec, frame);
    ASSERT_TRUE(ec == StompError::kOk);
}

TEST(TestStompFrameParsing, TestParseEmptyBody){

    std::string frame =
        "DISCONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "\0"
        ;

    StompError ec;
    StompFrame(ec, frame);
    ASSERT_TRUE(ec == StompError::kOk);
}

TEST(TestStompFrameParsing, TestParseInvalidCommand){

    std::string frame =
        "CONNECTO\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body\0";

    StompError ec;
    StompFrame(ec, frame);
    ASSERT_TRUE(ec == StompError::kInvalidCommand);
}

TEST(TestStompFrameParsing, TestParseInvalidHeaderKey){

    std::string frame =
        "CONNECT\n"
        "accept-versioning:42\n"
        "host:host.com\n"
        "\n"
        "Frame body\0";

    StompError ec;
    StompFrame(ec, frame);
    ASSERT_TRUE(ec == StompError::kInvalidHeaderKey);
}