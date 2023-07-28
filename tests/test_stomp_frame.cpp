#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>

#include "network-monitor/stomp-frame.h"

using namespace NetworkMonitor;
using namespace std::string_literals;

TEST(TestStompFrameParsing, TestParseWellFormed){

    std::string frame {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body\0"s
    };
    StompError ec;
    StompFrame(ec, frame);
    ASSERT_TRUE(ec == StompError::kOk);
}

TEST(TestStompFrameParsing, TestParseEmptyHeaders){

    std::string frame {
        "CONNECT\n"
        "\n"
        "Frame body\0"s
    };
    StompError ec;
    StompFrame(ec, std::move(frame));
    ASSERT_TRUE(ec == StompError::kOk);
}


TEST(TestStompFrameParsing, TestParseWellFormedContentLength){

    std::string frame =
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:10\n"
        "\n"
        "Frame body\0"s;

    StompError ec;
    StompFrame stompFrame {ec, frame};
    ASSERT_TRUE(ec == StompError::kOk);
    ASSERT_EQ(stompFrame.GetCommand(), StompCommand::kConnect);
    ASSERT_EQ(stompFrame.GetHeaderValue(StompHeader::kAcceptVersion), "42");
    ASSERT_EQ(stompFrame.GetHeaderValue(StompHeader::kContentLength), "10");
    ASSERT_EQ(stompFrame.GetCommand(), StompCommand::kConnect);

}

TEST(TestStompFrameParsing, TestContentLengthMismatch){

    std::string frame =
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:10\n"
        "\n"
        "Frame body11\0"s;

    StompError ec;
    StompFrame stompFrame {ec, frame};
    ASSERT_TRUE(ec == StompError::kConentLengthMistMatchError); 
}

TEST(TestStompFrameParsing, TestContentLengthMismatch2){

    std::string frame =
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:100\n"
        "\n"
        "Frame body11\0"s;

    StompError ec;
    StompFrame stompFrame {ec, frame};
    ASSERT_TRUE(ec == StompError::kConentLengthMistMatchError); 
}



TEST(TestStompFrameParsing, TestJunkAfterBodyNull){

    std::string frame =
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:10\n"
        "\n"
        "Frame body\0abc"s;

    StompError ec;
    StompFrame stompFrame {ec, frame};
    ASSERT_TRUE(ec == StompError::kParsingExtraJunkFoundAfterNullInBody); 
}



TEST(TestStompFrameParsing, TestParseEmptyBody){

    std::string frame =
        "DISCONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "\0"s
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
        "Frame body\0"s;

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
        "Frame body\0"s;

    StompError ec;
    StompFrame(ec, frame);
    ASSERT_TRUE(ec == StompError::kInvalidHeaderKey);
}

