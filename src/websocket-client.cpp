#include "network-monitor/websocket-client.h"
#include "logging.h"

void NetworkMonitor::WebSocketClient::Connect(
    std::function<void(boost::system::error_code)> onConnect,
    std::function<void(boost::system::error_code, std::string &&)> onMessage,
    std::function<void(boost::system::error_code)> onDisconnect)
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

void NetworkMonitor::WebSocketClient::Send(
    const std::string &message,
    std::function<void(boost::system::error_code)> onSend)
{
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

void NetworkMonitor::WebSocketClient::Close(
    std::function<void(boost::system::error_code)> onClose)
{
    logger::info("Close - start closing socket");
    callbacks_.onClose = onClose;
    websocket_.async_close(
        beast::websocket::close_code::normal,
        beast::bind_front_handler(&NetworkMonitor::WebSocketClient::OnClose, shared_from_this()));
}

NetworkMonitor::WebSocketClient::~WebSocketClient()
{
    return;
}
void NetworkMonitor::WebSocketClient::ListenToMessageOnBackground(beast::error_code const &ec)
{
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

// =========================================== LIFE CYLE FUNCTIONS ==============================================

void NetworkMonitor::WebSocketClient::OnResolve(beast::error_code ec, tcp::resolver::results_type results)
{
    logger::info("OnResolve Entry");
    if (ec)
    {
        logger::error("OnResolve - failed to resolve url: {} at port: {}", this->url_, this->port_);
        if (this->callbacks_.onConnect)
        {
            callbacks_.onConnect(ec);
        }
        logger::error("OnResolve Error Code: {}", ec.message());
        this->is_connected_ = false;
        return;
    }
    // get resolve data
    logger::info("OnResolve Set IP Connection Timeout to {} sec", IP_CONNECTION_TIMEOUT_SEC);
    beast::get_lowest_layer(websocket_).expires_after(std::chrono::seconds(IP_CONNECTION_TIMEOUT_SEC));
    // try to connect with given sequences of ip address
    beast::get_lowest_layer(websocket_).async_connect(*results.begin(), beast::bind_front_handler(&NetworkMonitor::WebSocketClient::OnConnect, shared_from_this()));
}

void NetworkMonitor::WebSocketClient::OnConnect(beast::error_code ec)
{
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
                                    websocket_.async_handshake(url_, endpoint_, beast::bind_front_handler(&NetworkMonitor::WebSocketClient::OnHandshake, shared_from_this())); 
                                });

}

void NetworkMonitor::WebSocketClient::OnHandshake(beast::error_code ec)
{
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

void NetworkMonitor::WebSocketClient::OnWriteComplete(beast::error_code const &ec, // Result of operation
                                                      std::size_t bytes_transferred)
{

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

void NetworkMonitor::WebSocketClient::OnReceivedMessage(beast::error_code const &ec, // Result of operation
                                                        std::size_t bytes_transferred)
{
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

void NetworkMonitor::WebSocketClient::OnClose(beast::error_code const &ec)
{
    logger::info("OnClose - Error Message: {}", ec ? ec.message() : "");
    this->is_connected_ = false;
    if (callbacks_.onClose)
    {
        callbacks_.onClose(ec);
    }
}