#include <string>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

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

    class WebSocketClient: public std::enable_shared_from_this<WebSocketClient> {
        private:
            const std::string& url_;
            const std::string& endpoint_;
            const std::string port_;
            const boost::asio::io_context& ioc_;
            boost::beast::websocket::stream<boost::beast::tcp_stream> websocket_;
            bool is_connected_ = false;
            tcp::resolver resolver_;
            RegisteredCallbacks callbacks_;
            beast::flat_buffer read_buffer_;

            // void OnReceivedMessage()

            // void OnWriteComplete()

            void OnResolve(beast::error_code ec, tcp::resolver::results_type results);

            void OnConnect(beast::error_code ec);

            void OnHandshake(beast::error_code ec);

            void OnWriteComplete(beast::error_code const& ec,           // Result of operation
                                 std::size_t bytes_transferred   // Number of bytes sent from the
                                );
            
            void OnReceivedMessage(beast::error_code const& ec,           // Result of operation
                                 std::size_t bytes_transferred   // Number of bytes sent from the
                                );

            void OnClose(beast::error_code const& ec);

            void ListenToMessageOnBackground(beast::error_code const& ec);

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
                boost::asio::io_context& ioc
            ): url_(url), endpoint_(endpoint), port_(port), ioc_(ioc), websocket_(net::make_strand(ioc)), resolver_(net::make_strand(ioc)){
                callbacks_.onConnect = nullptr;
                callbacks_.onDisconnect = nullptr;
                callbacks_.onMessage = nullptr;
                callbacks_.onSend = nullptr;
                callbacks_.onClose = nullptr;
            };

            /*! \brief WebSocketClient desstructor */
            ~WebSocketClient();

            /*! \brief Connect to a server 
            *
            * \param onConnect                Called when the connection is fail or success.
            * \param onMessage                Called when received a message successfully from Server.
            *                                 The message is an rvalue reference,; ownership passed to the receiver.
            * \param onDisconnect             Called when connection is closed by server or connection error
            * 
            */
            void Connect(
                std::function<void (boost::system::error_code)> onConnect = nullptr,
                std::function<void (boost::system::error_code, std::string&&)> onMessage = nullptr,
                std::function<void (boost::system::error_code)> onDisconnect = nullptr
            );

            /*! \brief Send a string message to remote server 
            *
            *  \param message         The messasge to send. The caller must make sure that this string stays in scope until onSend 
            *                         handler is called. 
            *  \param onSend          Called when a message is sent successfully or if it failed to send
            */
            void Send(
                const std::string& messsage,
                std::function<void (boost::system::error_code)> onSend = nullptr
            );

            /*! \brief Close the websocket connection.
            * \param onClose Called when the connection is closed successfully or ther is an error during close 
            */
            void Close(
                std::function<void (boost::system::error_code)> onClose = nullptr
            );
    };
}