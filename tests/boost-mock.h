
#include <string>
#include <boost/utility/string_view.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <logging.h>

using std::string;

class MockResolver {

    public: 
        template<typename ExecutionContext>
        explicit MockResolver(ExecutionContext&& ctx): context_(ctx) {}

        template <typename ResolveHandler>
        auto async_resolve(
            boost::string_view host,
            boost::string_view service,
            ResolveHandler&& handler
        )
        {
            using resolver = boost::asio::ip::tcp::resolver;
            return boost::asio::async_initiate<ResolveHandler, 
                                                void(const boost::system::error_code&, resolver::results_type)
            >(
               [](auto&& handler, auto resolver, string signature){
                    // call user handler here 
                    logger::info("{}", signature);
                    if (MockResolver::resolverErrorCode){
                        boost::asio::post(
                            resolver->context_,
                            boost::beast::bind_handler(
                                std::move(handler),
                                MockResolver::resolverErrorCode,
                                resolver::results_type{}));
                    }
                    else {
                        // do nothing here 
                    }
                },
               handler, 
               this,
               "mock resolver" 
            );

        }

        static boost::system::error_code resolverErrorCode;

    private:
        boost::asio::strand<boost::asio::io_context::executor_type> context_;
};

// out of line static member initialization 

inline boost::system::error_code MockResolver::resolverErrorCode = {};

