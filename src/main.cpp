#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <thread>
#include <iomanip>
#include <vector>

using tcp = boost::asio::ip::tcp;
using ip_address = boost::asio::ip::address;
using boost_error_code = boost::system::error_code;

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
   tcp::socket socket {boost::asio::make_strand(ioc)};
   tcp::resolver resolver {ioc};
   boost::asio::ip::basic_resolver_results results {resolver.resolve("google.com", "80", ec)};
   
   size_t nThreads = 10;

   std::vector<std::thread> tPools;
   tPools.reserve(nThreads);

   for (int i = 0; i < nThreads; i++){
      // create bunch of async jobs
      socket.async_connect(*results.begin(), OnConnect);
   }
   for (size_t i = 0; i < nThreads; i++){
      tPools.emplace_back(
         [&ioc](void){
         // std::cout << "["
         //           << std::setw(14) << std::this_thread::get_id()
         //           << "] ioc.run()"
         //           << std::endl;
         ioc.run();
      }
      );
   }
   // call ioc.run() on a thread 
   for (auto& t: tPools){
      t.join();
   }
   
   return 0;

}
