
#include <string>
#include <boost/utility/string_view.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>

#include <logging.h>

#pragma once

using std::string;

namespace NetworkMonitor
{

    class MockResolver
    {

    public:
        template <typename ExecutionContext>
        explicit MockResolver(ExecutionContext &&ctx) : context_(ctx) {}

        template <typename ResolveHandler>
        auto async_resolve(
            boost::string_view host,
            boost::string_view service,
            ResolveHandler &&handler)
        {
            using resolver = boost::asio::ip::tcp::resolver;
            return boost::asio::async_initiate<ResolveHandler,
                                               void(const boost::system::error_code &, resolver::results_type)>(
                [](auto &&handler, auto resolver, string host, string service, string signature)
                {
                    // call user handler here
                    logger::info("{}", signature);
                    if (MockResolver::resolverErrorCode)
                    {
                        boost::asio::post(
                            resolver->context_,
                            boost::beast::bind_handler(
                                std::move(handler),
                                MockResolver::resolverErrorCode,
                                resolver::results_type{}));
                    }
                    else
                    {
                        boost::asio::post(
                            resolver->context_,
                            boost::beast::bind_handler(
                                std::move(handler),
                                MockResolver::resolverErrorCode,
                                resolver::results_type::create(
                                    boost::asio::ip::tcp::endpoint {
                                        boost::asio::ip::make_address(
                                            "127.0.0.1"
                                        ),
                                        443

                                    },
                                    host,
                                    service
                                ) 
                            )
                        );
                    }
                },
                handler,
                this,
                host.to_string(),
                service.to_string(),
                "mock resolver");
        }

        static boost::system::error_code resolverErrorCode;

    private:
        boost::asio::strand<boost::asio::io_context::executor_type> context_;
    };

    // out of line static member initialization

    inline boost::system::error_code MockResolver::resolverErrorCode = {};


    class MockTcpStream: public boost::beast::tcp_stream {
    public:
        /* inherit all of tcp_stream constructor */
        using boost::beast::tcp_stream::tcp_stream;

        static boost::system::error_code connectErrorCode;

        template <typename ConnectHandler> 
        auto async_connect(
            endpoint_type const &ep,
            ConnectHandler &&handler)
        {
            return boost::asio::async_initiate<ConnectHandler,
                                               void(boost::system::error_code)>(
                [](auto &&handler, auto stream)
                {
                    boost::asio::post(
                        stream->get_executor(),
                        boost::beast::bind_handler(
                            std::move(handler),
                            MockTcpStream::connectErrorCode));
                },
                handler,
                this);
        }

    };

    /* allocate space for static varible */
    inline boost::system::error_code MockTcpStream::connectErrorCode {};

    // need to overload teardown as it is required by boost
    // This overload is required by Boost.Beast when you define a custom stream.
    template <typename TeardownHandler>
    void async_teardown(
        boost::beast::role_type role,
        MockTcpStream &socket,
        TeardownHandler &&handler)
    {
        return;
    }


    template <typename TcpStreamLayer>
    class MockSslStream: public boost::beast::ssl_stream<TcpStreamLayer> {
        public:
            /* inherit all ssl_stream constructor
               the base class is now boost::beast::ssl_stream<boost::beast::tcp_stream> 
            */
            using boost::beast::ssl_stream<TcpStreamLayer>::ssl_stream;

            static boost::system::error_code sslErrorCode;

            template<typename HandshakeHandler>
            auto async_handshake(boost::asio::ssl::stream_base::handshake_type type, HandshakeHandler &&handler){
                
                  return boost::asio::async_initiate<HandshakeHandler,
                                                     void (boost::system::error_code)> 
                  (
                    [](HandshakeHandler &&handler, auto stream) {
                         boost::asio::post(
                            stream->get_executor(),
                            boost::beast::bind_handler(
                               std::move(handler),
                               MockSslStream::sslErrorCode
                            )
                         );
                    },
                    handler,
                    this 
                  );

            }



    };

    template <typename TcpStreamLayer>
    inline boost::system::error_code MockSslStream<TcpStreamLayer>::sslErrorCode = {};

    template <typename TransportLayer>
    class MockWebSocketStream: public boost::beast::websocket::stream<TransportLayer> {

        public:
            using boost::beast::websocket::stream<TransportLayer>::stream;
            static boost::system::error_code websocketErrorCode;
            
            template<typename HandshakeHandler>
            auto async_handshake(
                boost::string_view host,
                boost::string_view target,
                HandshakeHandler&& handler){

                boost::asio::async_initiate<HandshakeHandler,
                                            void (const boost::beast::error_code &)>(

                    [](auto &&handler, auto stream){
                         boost::asio::post(
                            stream->get_executor(),
                            boost::beast::bind_handler(
                                std::move(handler),
                                MockWebSocketStream::websocketErrorCode
                            )
                         );
                    },
                    handler,
                    this 
                );
            }

    };

    template <typename TransportLayer>
    inline boost::system::error_code MockWebSocketStream<TransportLayer>::websocketErrorCode = {};

    template<typename TeardownHandler, typename TransportLayer>
    void
    async_teardown(
        boost::beast::role_type role,
        MockWebSocketStream<TransportLayer>& stream,
        TeardownHandler&& handler)
    {
        return;
    }

    template<class TeardownHandler, typename TransportLayer>
    void
    async_teardown(
        boost::beast::role_type role,
        MockSslStream<TransportLayer>& stream,
        TeardownHandler&& handler)
    {
        return;
    }

}