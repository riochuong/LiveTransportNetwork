#include <string>
#include <memory>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

#include <logging.h>

namespace net = boost::asio;  
namespace beast = boost::beast;
using tcp = boost::asio::ip::tcp;


#define IP_CONNECTION_TIMEOUT_SEC 60

namespace NetworkMonitor{
    /* ! \brief WebClientSocket inorder to connect o a server*/
    typedef struct {
        std::function<void (boost::system::error_code)> onConnect;
        std::function<void (boost::system::error_code, std::string&&)> onMessage;
        std::function<void (boost::system::error_code)> onDisconnect;
        std::function<void (boost::system::error_code)> onSend;
        std::function<void (boost::system::error_code)> onClose;
    } RegisteredCallbacks;

    template<
        typename Resolver,
        typename WebSocketStream
    >
    class WebSocketClient: public std::enable_shared_from_this<WebSocketClient> {
        private:
            const std::string& url_;
            const std::string& endpoint_;
            const std::string port_;
            const boost::asio::io_context& ioc_;
            bool is_connected_ = false;
            Resolver resolver_;
            RegisteredCallbacks callbacks_;
            beast::flat_buffer read_buffer_;
            WebSocketStream websocket_;
            // void OnReceivedMessage()

            // void OnWriteComplete()

            void OnResolve(beast::error_code ec, tcp::resolver::results_type results);

            void OnConnect(beast::error_code ec) {
                logger::info("OnConnect Entry");
                if (ec)
                {
                    logger::error("OnConnect - Failed to Connect with Error Code: {}", ec.message());
                    if (this->callbacks_.onConnect)
                    {
                        callbacks_.onConnect(ec);
                    }
                    this->is_connected_ = false;
                    return;
                }
                logger::info("OnConnect - Successfully Estsablished Connection. Starting Handshake");
                // disable timeout for tcp layer as websocket stream has its own timeout
                beast::get_lowest_layer(websocket_).expires_never();
                // set client timeout basedon boost recommendation
                websocket_.set_option(beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));
                // Some clients require that we set the host name before the TLS handshake
                // or the connection will fail. We use an OpenSSL function for that.
                SSL_set_tlsext_host_name(websocket_.next_layer().native_handle(), url_.c_str());
                websocket_.next_layer().async_handshake(net::ssl::stream_base::handshake_type::client, [this](const boost::system::error_code &ec)
                                                        {
                                    if (ec)
                                    {
                                        logger::error("OnConnect - Failed TLS handshake: {}", ec.message());
                                        if (this->callbacks_.onConnect)
                                        {
                                            callbacks_.onConnect(ec);
                                        }
                                        this->is_connected_ = false;
                                        return;
                                   }
                                    logger::error("OnConnect - Successfully TLS handhsake");
                                    websocket_.async_handshake(url_, endpoint_, beast::bind_front_handler(&NetworkMonitor::WebSocketClient::OnHandshake, shared_from_this())); });
            }

            void OnHandshake(beast::error_code ec){
                logger::info("OnHandshake Entry");
                if (ec)
                {
                    logger::error("OnHandshake - Failed to Handshake with Error Code: %s", ec.message());
                    if (this->callbacks_.onConnect)
                    {
                        callbacks_.onConnect(ec);
                    }
                    this->is_connected_ = false;
                    return;
                }
                logger::info("OnHandshake - Successfully established connection with remote server ");
                this->is_connected_ = true;
                if (this->callbacks_.onConnect)
                {
                    callbacks_.onConnect(ec);
                }
                this->ListenToMessageOnBackground(ec);
            }

            void OnWriteComplete(beast::error_code const& ec,           // Result of operation
                                 std::size_t bytes_transferred   // Number of bytes sent from the
                                ){
                logger::info("OnWritecomplete Etnry");
                if (ec)
                {
                    logger::error("OnWritecomplete - Failed to write with Error Code: {}", ec.message());
                    if (this->callbacks_.onSend)
                    {
                        callbacks_.onSend(ec);
                    }
                    return;
                }
                logger::info("OnWritecomplete - Successfully sent message of {} bytes", bytes_transferred);
                if (this->callbacks_.onSend)
                {
                    callbacks_.onSend(ec);
                }
            }

            void OnReceivedMessage(beast::error_code const& ec,           // Result of operation
                                 std::size_t bytes_transferred   // Number of bytes sent from the
                                ){
                logger::info("OnReceivedMessage");
                if (ec)
                {
                    logger::error("OnReceivedMessage - Failed to received message with Error Code: {}", ec.message());
                    if (!is_connected_)
                    {
                        logger::warn("OnReceivedMessage - disconnected");
                        return;
                    }

                    if (this->callbacks_.onMessage)
                    {
                        callbacks_.onMessage(ec, "");
                    }
                    return;
                }
                logger::info("OnReceivedMessage - Successfully received message from remote");
                if (callbacks_.onMessage)
                {
                    callbacks_.onMessage(ec, beast::buffers_to_string(read_buffer_.data()));
                }
                this->ListenToMessageOnBackground(ec);
            }

            void OnClose(beast::error_code const& ec){
                logger::info("OnClose - Error Message: {}", ec ? ec.message() : "");
                this->is_connected_ = false;
                if (callbacks_.onClose)
                {
                    callbacks_.onClose(ec);
                }
            }

            void ListenToMessageOnBackground(beast::error_code const& ec){
                logger::info("Kicked off background listening to incoming message");
                if (!this->is_connected_ || ec)
                {
                    if (callbacks_.onDisconnect && ec == boost::asio::error::operation_aborted)
                    {
                        callbacks_.onDisconnect(ec);
                    }
                    logger::error("Failed to start listening to background message due to {}", ec.message());
                    return;
                }
                websocket_.async_read(read_buffer_,
                                      beast::bind_front_handler(
                                          &NetworkMonitor::WebSocketClient::OnReceivedMessage,
                                          shared_from_this()));
            }

        public:
        
            /*! \brief Construct a Websocket Client
            * \note This constructor does not initiate a connection 
            *
            * \param url       The URL of the server
            * \param endpoint  The endpoint of the server to connect to.
            *                  Example: ltnm.learncppthroughprojects.com/<endpoint>
            * \param port      The port on the server
            * \param ioc       The io_context object. The user takes care of calling ioc.run()
            * 
            */
            WebSocketClient(
                const std::string& url,
                const std::string& endpoint,
                const std::string& port,
                boost::asio::io_context& ioc,
                net::ssl::context& ssl_context
            ): url_(url), 
               endpoint_(endpoint), 
               port_(port), 
               ioc_(ioc), 
               resolver_(net::make_strand(ioc)),
               websocket_(net::make_strand(ioc), ssl_context) 
               {
                callbacks_.onConnect = nullptr;
                callbacks_.onDisconnect = nullptr;
                callbacks_.onMessage = nullptr;
                callbacks_.onSend = nullptr;
                callbacks_.onClose = nullptr;
            };

            /*! \brief WebSocketClient desstructor */
            ~WebSocketClient(){}

            /*! \brief Connect to a server 
            *
            * \param onConnect                Called when the connection is fail or success.
            * \param onMessage                Called when received a message successfully from Server.
            *                                 The message is an rvalue reference,; ownership passed to the receiver.
            * \param onDisconnect             Called when connection is closed by server or connection error
            * 
            */
            void Connect(
                std::function<void(boost::system::error_code)> onConnect = nullptr,
                std::function<void(boost::system::error_code, std::string &&)> onMessage = nullptr,
                std::function<void(boost::system::error_code)> onDisconnect = nullptr)
            {

                if (this->is_connected_)
                {
                    logger::warn("Socket is already connected ! Please close before trying to reconnect !");
                    return;
                }
                this->callbacks_.onConnect = onConnect;
                this->callbacks_.onMessage = onMessage;
                this->callbacks_.onDisconnect = onDisconnect;

                resolver_.async_resolve(
                    this->url_,
                    this->port_,
                    beast::bind_front_handler(
                        &NetworkMonitor::WebSocketClient::OnResolve,
                        shared_from_this()));
            }

            /*! \brief Send a string message to remote server
             *
             *  \param message         The messasge to send. The caller must make sure that this string stays in scope until onSend
             *                         handler is called.
             *  \param onSend          Called when a message is sent successfully or if it failed to send
             */
            void Send(
                const std::string& messsage,
                std::function<void (boost::system::error_code)> onSend = nullptr
            ){
                if (!this->is_connected_)
                {
                    logger::error("Connection is not yet established !!! Please start connect or retry later");
                    return;
                }
                callbacks_.onSend = onSend;
                websocket_.async_write(net::buffer(message),
                                       beast::bind_front_handler(&NetworkMonitor::WebSocketClient::OnWriteComplete,
                                                                 shared_from_this()));
            }

            /*! \brief Close the websocket connection.
            * \param onClose Called when the connection is closed successfully or ther is an error during close 
            */
            void Close(
                std::function<void (boost::system::error_code)> onClose = nullptr
            ){
                logger::info("Close - start closing socket");
                callbacks_.onClose = onClose;
                websocket_.async_close(
                    beast::websocket::close_code::normal,
                    beast::bind_front_handler(&NetworkMonitor::WebSocketClient::OnClose, shared_from_this()));
            }
    };
    using BoostWebSocketClient = WebSocketClient<
        boost::asio::ip::tcp::resolver,
        boost::beast::websocket::stream<
            boost::beast::ssl_stream<boost::beast::tcp_stream>>>;
}