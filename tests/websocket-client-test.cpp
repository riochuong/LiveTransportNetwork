#include <iostream>
#include <string>
#include <filesystem>    // only available C++17
#include <sstream>
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

TEST(WebSocketClientTestSuite, TestStompNetworkEvents){
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    std::stringstream ss;
    ss << "STOMP\n";
    ss << "accept-version:1.2\n";
    ss << "host:transportforlondon.com\n";
    ss << "login:madeup\n";
    ss << "passcode:madeup\n";
    ss << "\n";
    ss << '\0'; // The body (even absent) must be followed by a NULL octet
    net::io_context ioc {};
    net::ssl::context ssl_context {net::ssl::context::tlsv12_client};
    ssl_context.set_verify_mode(net::ssl::verify_peer);
    ssl_context.load_verify_file(TEST_CACERT_PEM);
    // need to created shared_pointer here to enable share_from_this
    auto client_ptr = std::make_shared<WebSocketClient>(url, endpoint, port, ioc, ssl_context);

    std::function<bool(std::string)> check_response = [](std::string response)
    {
        bool ok{true};
        ok &= response.find("ERROR") != std::string::npos;
        ok &= response.find("ValidationInvalidAuth") != std::string::npos;
        return ok;
    };

    auto onSend{[](auto ec)
                {
                    ASSERT_TRUE(!ec);
                }};
    auto onConnect{[&client_ptr, &onSend, &ss](auto ec)
                   {
                       ASSERT_TRUE(!ec);
                       if (!ec)
                       {
                           client_ptr->Send(ss.str(), onSend);
                       }
                   }};
    
    auto onClose{[](auto ec)
                 {
                     ASSERT_TRUE(!ec);
                     logger::info("Websocket Closed ");
                 }};

    auto onReceive{[&client_ptr,
                    &check_response,
                    &onClose](auto ec, auto received)
                   {
                       ASSERT_TRUE(!ec);
                       logger::info("Message Matches: received {}", received);
                       ASSERT_TRUE(check_response(received));
                       client_ptr->Close(onClose);
                   }};

    client_ptr->Connect(onConnect, onReceive);
    ioc.run();
}

int main(void)
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}