#include <iostream>
#include <string>
#include <filesystem>    // only available C++17
#include <memory>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <gtest/gtest.h>
#include <boost/asio.hpp>

#include "network-monitor/websocket-client.h"
#include "logging.h"
#include "boost-mock.h"

using TestWebSocketClient = NetworkMonitor::WebSocketClient<
    NetworkMonitor::MockResolver,
    NetworkMonitor::MockWebSocketStream<
        NetworkMonitor::MockSslStream<NetworkMonitor::MockTcpStream>
    >
>;

namespace net = boost::asio;  
using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;
using NetworkMonitor::BoostWebSocketClient;
using namespace NetworkMonitor;

class TestWebSocketClientFixture: public ::testing::Test {

    protected:
        void SetUp() {
            NetworkMonitor::MockResolver::resolverErrorCode = {};
            NetworkMonitor::MockTcpStream::connectErrorCode = {};
            NetworkMonitor::MockSslStream<NetworkMonitor::MockTcpStream>::sslErrorCode = {};
            NetworkMonitor::MockWebSocketStream<
                    NetworkMonitor::MockSslStream<NetworkMonitor::MockTcpStream>>::websocketErrorCode = {};
        }

        void TearDown() {

        }
};

TEST_F(TestWebSocketClientFixture, TestWebsocketFailedHandshake){
    
    const std::string url {"some.echo-server.com"};
    const std::string endpoint{"/"};
    const std::string port {"443"};

    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);
    boost::asio::io_context ioc {};
    namespace websocket = boost::beast::websocket;
    NetworkMonitor::MockWebSocketStream<
                    NetworkMonitor::MockSslStream<NetworkMonitor::MockTcpStream>>::websocketErrorCode = websocket::error::no_connection;

    auto clientPtr = std::make_shared<TestWebSocketClient>(url, endpoint, port, ioc, ctx);

    bool on_connect_called = false;
    auto OnConnect {
        [&on_connect_called](auto ec){
            ASSERT_EQ(ec, websocket::error::no_connection);
            on_connect_called = true;
        }
    };
    clientPtr->Connect(OnConnect);
    ioc.run();
    ASSERT_TRUE(on_connect_called);

}

TEST_F(TestWebSocketClientFixture, TestSSLFailedHandshake){
    
    const std::string url {"some.echo-server.com"};
    const std::string endpoint{"/"};
    const std::string port {"443"};

    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);
    boost::asio::io_context ioc {};
    namespace ssl_error = boost::asio::ssl::error;
    NetworkMonitor::MockSslStream<NetworkMonitor::MockTcpStream>::sslErrorCode = ssl_error::stream_truncated;

    auto clientPtr = std::make_shared<TestWebSocketClient>(url, endpoint, port, ioc, ctx);

    bool on_connect_called = false;
    auto OnConnect {
        [&on_connect_called](auto ec){
            ASSERT_EQ(ec, ssl_error::stream_truncated);
            on_connect_called = true;
        }
    };
    clientPtr->Connect(OnConnect);
    ioc.run();
    ASSERT_TRUE(on_connect_called);

}

TEST_F(TestWebSocketClientFixture, TestDNSFailed){
    
    const std::string url {"some.echo-server.com"};
    const std::string endpoint{"/"};
    const std::string port {"443"};

    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);
    boost::asio::io_context ioc {};
    NetworkMonitor::MockResolver::resolverErrorCode = boost::asio::error::host_not_found;

    auto clientPtr = std::make_shared<TestWebSocketClient>(url, endpoint, port, ioc, ctx);

    bool on_connect_called = false;
    auto OnConnect {
        [&on_connect_called](auto ec){
            ASSERT_EQ(ec, boost::asio::error::host_not_found);
            on_connect_called = true;
        }
    };
    clientPtr->Connect(OnConnect);
    ioc.run();
    ASSERT_TRUE(on_connect_called);

}

TEST_F(TestWebSocketClientFixture, TestOnMessageReceived){
    
    const std::string url {"some.echo-server.com"};
    const std::string endpoint{"/"};
    const std::string port {"443"};

    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);
    boost::asio::io_context ioc {}; 

    auto clientPtr = std::make_shared<TestWebSocketClient>(url, endpoint, port, ioc, ctx);
    using namespace NetworkMonitor;
    NetworkMonitor::MockWebSocketStream<
                    NetworkMonitor::MockSslStream<NetworkMonitor::MockTcpStream>>::readMessage = {"test 123"};
    bool on_connect_called = false;
    bool on_message_called = false;
    auto OnConnect {
        [&on_connect_called](auto ec){
            on_connect_called = true;
        }
    };
    auto OnMessage {
        [&on_message_called](auto ec, string message){
            namespace websocket = boost::beast::websocket;
            if (on_message_called){
                ASSERT_TRUE(ec == websocket::error::bad_method);
                return;
            }
            on_message_called = true;
            // set error here so we only reach here once 
            MockWebSocketStream<MockSslStream<MockTcpStream>>::websocketErrorCode = websocket::error::bad_method; 
            ASSERT_TRUE(message == MockWebSocketStream<MockSslStream<MockTcpStream>>::readMessage);
        }
    };
    clientPtr->Connect(OnConnect, OnMessage);
    ioc.run();
    ASSERT_TRUE(on_connect_called);
    ASSERT_TRUE(on_message_called);
    clientPtr->Close();
}

TEST_F(TestWebSocketClientFixture, TestMockOnSendMessage){
    
    const std::string url {"some.echo-server.com"};
    const std::string endpoint{"/"};
    const std::string port {"443"};

    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);
    boost::asio::io_context ioc {}; 

    auto clientPtr = std::make_shared<TestWebSocketClient>(url, endpoint, port, ioc, ctx);
    using namespace NetworkMonitor;
    NetworkMonitor::MockWebSocketStream<
                    NetworkMonitor::MockSslStream<NetworkMonitor::MockTcpStream>>::readMessage = {"test 123"};
    bool on_connect_called = false;
    bool on_send_called = false;
    std::string sentMessage = "test mock 123";

    // stop looping of reading 
    auto OnMessage{
        [](auto ec, std::string msg)
        {
            
            MockWebSocketStream<MockSslStream<MockTcpStream>>::websocketErrorCode = \
                            boost::asio::error::operation_aborted; 
        }
    };

    auto OnSend {
        [&on_send_called, &sentMessage](auto ec, std::size_t byte_transferred){
           on_send_called = true; 
           ASSERT_TRUE(sentMessage.size() == byte_transferred);
        }
    };

    auto OnConnect {
        [&on_connect_called, &sentMessage, clientPtr, &OnSend](auto ec){
            on_connect_called = true;
            clientPtr->Send(sentMessage, OnSend);
        }
    };
    clientPtr->Connect(OnConnect, OnMessage);
    ioc.run();
    ASSERT_TRUE(on_connect_called);
    ASSERT_TRUE(on_send_called);
    clientPtr->Close();
}



TEST_F(TestWebSocketClientFixture, TestFailedSocketConnect){
    const std::string url{"some.echo-server.com"};
    const std::string endpoint{"/"};
    const std::string port{"443"};

    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);
    boost::asio::io_context ioc{};
    NetworkMonitor::MockTcpStream::connectErrorCode = boost::asio::error::connection_refused;

    auto clientPtr = std::make_shared<TestWebSocketClient>(url, endpoint, port, ioc, ctx);

    bool on_connect_called = false;

    MockWebSocketStream<MockSslStream<MockTcpStream>>::websocketErrorCode = boost::asio::error::operation_aborted; 

    auto OnConnect{
        [&on_connect_called](auto ec)
        {
            ASSERT_EQ(ec, boost::asio::error::connection_refused);
            on_connect_called = true;
        }
    };

    clientPtr->Connect(OnConnect);
    ioc.run();
    ASSERT_TRUE(on_connect_called);
}

TEST_F(TestWebSocketClientFixture, TestMockSocketSuccessfullConnect){
    const std::string url{"some.echo-server.com"};
    const std::string endpoint{"/"};
    const std::string port{"443"};

    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);
    boost::asio::io_context ioc{};
    NetworkMonitor::MockTcpStream::connectErrorCode = {}; 
    auto clientPtr = std::make_shared<TestWebSocketClient>(url, endpoint, port, ioc, ctx);

    bool on_connect_called = false;
    auto OnConnect{
        [&on_connect_called](auto ec)
        {
            on_connect_called = true;
            MockWebSocketStream<MockSslStream<MockTcpStream>>::websocketErrorCode = boost::asio::error::operation_aborted; 
        }
    };
    clientPtr->Connect(OnConnect);
    ioc.run();
    ASSERT_TRUE(on_connect_called);
}

TEST_F(TestWebSocketClientFixture, TestMockSocketOnClose){
    const std::string url{"some.echo-server.com"};
    const std::string endpoint{"/"};
    const std::string port{"443"};

    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);
    boost::asio::io_context ioc{};
    NetworkMonitor::MockTcpStream::connectErrorCode = {}; 
    auto clientPtr = std::make_shared<TestWebSocketClient>(url, endpoint, port, ioc, ctx);

    bool on_connect_called = false;
    bool on_close_called = false;
    auto OnClose {
        [&on_close_called](auto ec){
            on_close_called = true;
        }
    };

    auto OnConnect{
        [&on_connect_called, clientPtr, &OnClose](auto ec)
        {
            on_connect_called = true;
            clientPtr->Close(OnClose);
        }
    };
    clientPtr->Connect(OnConnect);
    ioc.run();
    ASSERT_TRUE(on_connect_called);
}

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
    auto client_ptr = std::make_shared<BoostWebSocketClient>(url, endpoint, port, ioc, ssl_context);

    // We use these flags to check that the connection, send, receive functions
    // work as expected.
    bool connected {false};
    bool messageSent {false};
    bool messageReceived {false};
    bool messageMatches {false};
    bool disconnected {false};

    // Our own callbacks
    auto onSend {[&messageSent](auto ec, std::size_t byte_transferred) {
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
    auto client_ptr = std::make_shared<BoostWebSocketClient>(url, endpoint, port, ioc, ssl_context);

    std::function<bool(std::string)> check_response = [](std::string response)
    {
        bool ok{true};
        ok &= response.find("ERROR") != std::string::npos;
        ok &= response.find("ValidationInvalidAuth") != std::string::npos;
        return ok;
    };

    auto onSend{[](auto ec, std::size_t byte_transferred)
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