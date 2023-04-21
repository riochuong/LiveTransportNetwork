#include <iostream>
#include <thread>
#include <iomanip>
#include <vector>
#include <string>
#include <cassert>

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

void OnConnect(boost_error_code ec) {
   Log(ec);
}

int main (void) {
   boost::system::error_code ec {};
   boost::asio::io_context ioc{};

   // make strand to serialize execution of handlers 
   tcp::socket socket {ioc};
   tcp::resolver resolver {ioc};
   boost::asio::ip::basic_resolver_results results {resolver.resolve("ltnm.learncppthroughprojects.com", "80", ec)};
   socket.connect(*results.begin());
   stream<tcp_stream> websocket(std::move(socket));
   // communicate via text format 
   websocket.text(true);
   websocket.handshake("ltnm.learncppthroughprojects.com", "/echo", ec);
   Log(ec);
   if (ec){
      return -1;
   }
   ec.clear();
   std::string msg_orig = "Test 123";
   size_t sent_bytes = websocket.write(net::buffer(msg_orig));
   std::cout << "Sent bytes: " << sent_bytes <<"\n";
   std::string echo_msg;
   boost::asio::dynamic_string_buffer buffer{echo_msg};
   try{
      int num_bytes = websocket.read(buffer, ec);
      if (ec) {
         std::cerr << "Error reading data: " << ec.message() <<"\n";
         return -3;
      }
      std::cout << "Read bytes: " << num_bytes << "\n";
      assert(num_bytes == msg_orig.size());
      std::cout << "Echo message: " << echo_msg << std::endl;
      return 0;
   } catch (boost::system::system_error e){
      std::cerr << "Error: " << e.what() << "\n";
   };
   return -1;
}
