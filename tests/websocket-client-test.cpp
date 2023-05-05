#include <iostream>
#include <string>
#include <filesystem>    // only available C++17
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <gtest/gtest.h>


#include "network-monitor/websocket-client.h"
#include "logging.h"


namespace net = boost::asio;  
using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;
using NetworkMonitor::WebSocketClient;


TEST(WebSocketClientTestSuite, TestBasicWebsocketConnection) {
    
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/echo"};
    const std::string port {"443"};
    const std::string message {"Hello WebSocket"};
    net::io_context ioc {};
    net::ssl::context ssl_context {net::ssl::context::tlsv12_client};
    ssl_context.set_verify_mode(net::ssl::verify_peer);
    ssl_context.load_verify_file(TEST_CACERT_PEM);
    // need to created shared_pointer here to enable share_from_this
    auto client_ptr = std::make_shared<WebSocketClient>(url, endpoint, port, ioc, ssl_context);

    // We use these flags to check that the connection, send, receive functions
    // work as expected.
    bool connected {false};
    bool messageSent {false};
    bool messageReceived {false};
    bool messageMatches {false};
    bool disconnected {false};

    // Our own callbacks
    auto onSend {[&messageSent](auto ec) {
        messageSent = !ec;
    }};
    auto onConnect {[&client_ptr, &connected, &onSend, &message](auto ec) {
        connected = !ec;
        if (!ec) {
            client_ptr->Send(message, onSend);
        }
    }};
    auto onClose {[&disconnected](auto ec) {
        disconnected = !ec;
        logger::info("Websocket Closed: {}", disconnected);
    }};

    auto onReceive {[&client_ptr,
                      &onClose,
                      &messageReceived,
                      &messageMatches,
                      &message](auto ec, auto received) {
        messageReceived = !ec;
        messageMatches = message == received;
        logger::info("Message Matches: {} message {} received {}", messageMatches, message, received);
        client_ptr->Close(onClose);
    }};

    client_ptr->Connect(onConnect, onReceive);
    ioc.run();

    // When we get here, the io_context::run function has run out of work to do.
    bool ok {
        connected &&
        messageSent &&
        messageReceived &&
        messageMatches &&
        disconnected
    };
  ASSERT_TRUE(ok);
}

TEST(WebSocketClientTestSuite, TestCacertPem){
   ASSERT_TRUE(std::filesystem::exists(TEST_CACERT_PEM));     
}

int main(void) {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}