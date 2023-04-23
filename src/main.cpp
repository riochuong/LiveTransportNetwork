#include <iostream>
#include <thread>
#include <iomanip>
#include <vector>
#include <string>
#include <cassert>
#include <memory>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/beast.hpp>

//#include <boost/beast/ssl.hpp>


using tcp = boost::asio::ip::tcp;
using ip_address = boost::asio::ip::address;
using boost_error_code = boost::system::error_code;
namespace net = boost::asio;
using namespace boost::beast;
using namespace boost::beast::websocket;

void Log(boost_error_code ec) {
   std::cerr << (ec ? "Error : " : "OK")
             << (ec ? ec.message(): "")
             << std::endl;
}


int main (void) {
   boost::system::error_code ec {};
   boost::asio::io_context ioc{};

   // make strand to serialize execution of handlers 
   tcp::socket socket {ioc};
   tcp::resolver resolver {ioc};
   std::shared_ptr<stream<tcp_stream>> websocket_ptr = nullptr;
   std::string echo_msg;
   boost::asio::dynamic_string_buffer echo_buffer{echo_msg};
   
   boost::asio::ip::basic_resolver_results results {resolver.resolve("ltnm.learncppthroughprojects.com", "80", ec)};
   socket.async_connect(*results.begin(), [&websocket_ptr, &socket, &echo_buffer, &echo_msg](const boost_error_code &ec){
        if (ec) {
           Log(ec);
           return;
        }
        std::cout << "Successfully Connect  \n";
        websocket_ptr = std::make_shared<stream<tcp_stream>>(std::move(socket));
        assert(websocket_ptr);
        websocket_ptr->text(true);

        // Handshake and trigger write 
        websocket_ptr->async_handshake("ltnm.learncppthroughprojects.com", "/echo", [&websocket_ptr, &echo_buffer, &echo_msg](const boost_error_code &ec){
            if (ec){
               Log(ec);
               return;
            }
            std::cout << "Successfully Established Websocket Handshake  \n";
            std::string msg_orig = "Test 123 ---- today is a great day !! Space X Starship is succesfully lifted off the base";
            websocket_ptr->async_write(net::buffer(msg_orig), [&websocket_ptr, &echo_buffer, &echo_msg](const boost_error_code& ec, size_t bytes_transferred){
                  if (ec){
                     Log(ec);
                     return;
                  }
                  assert(websocket_ptr);
                  std::cout << "Sent bytes: " << bytes_transferred <<"\n";
                  // now do async read
                  websocket_ptr->async_read(echo_buffer, [&websocket_ptr, &echo_buffer, bytes_transferred, &echo_msg](const boost_error_code& ec, size_t bytes_written){
                        if (ec){
                           Log(ec);
                           return;
                        }
                        std::cout << "Bytes Written :" << bytes_written << "\n";
                        assert(websocket_ptr);
                        assert(bytes_transferred == bytes_written);
                        std::cout << "Received message: " << echo_msg << "\n";
                     });
            });
        });
   });
   ioc.run();
   return 0;        
}
