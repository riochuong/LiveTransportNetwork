#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>

int main (void) {
   boost::system::error_code ec {};
   if (ec) {
      std::cerr << "Error: " << ec.message() << std::endl;
      return -1;
   } else {
      std::cout << "It works" << std::endl;
      return 0;   
   }
}
