#pragma once

#include <string_view>
#include <chrono>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/log/trivial.hpp>     // для BOOST_LOG_TRIVIAL
#include <boost/log/core.hpp>        // для logging::core
#include <boost/log/expressions.hpp> // для выражения, задающего фильтр
#include <boost/log/utility/setup/console.hpp>
#include "http_server.h"
#include "logger.h"

namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace beast = boost::beast;
namespace http = beast::http;
namespace logging = boost::log;
namespace keywords = boost::log::keywords;

namespace logging_handler {
namespace detail {
class DurationMeasure {
public:
    DurationMeasure() = default;

    std::chrono::steady_clock::duration GetDuration() const {
        std::chrono::steady_clock::time_point end_ts = std::chrono::steady_clock::now();
        return end_ts - start_ts_;
    }

private:
    std::chrono::steady_clock::time_point start_ts_ = std::chrono::steady_clock::now();
};
} // namespace detail

using namespace std::string_literals;

template <class RequestHandler>
class LoggingRequestHandler {
public:
   explicit LoggingRequestHandler(RequestHandler& request_handler)
        : request_handler_(request_handler) {}

    template <typename Body, typename Allocator, typename Send>
    void operator()(const boost::asio::ip::tcp::endpoint& endpoint, http::request<Body, 
			http::basic_fields<Allocator>>&& req, Send&& send) {
        
	using ReqType = http::request<Body, http::basic_fields<Allocator>>;
        
		json_logger::JsonLogger::GetInstance().LogRequest(endpoint, req);

        detail::DurationMeasure dur_measure;

        request_handler_ (endpoint, std::move(req), [s = std::forward<Send>(send), 
						     d = std::forward<detail::DurationMeasure>(dur_measure)
						    ] (auto&& response) {
            json_logger::JsonLogger::GetInstance().LogResponse(
                        d.GetDuration(),
                        static_cast<int>(response.result()),
                       response.at(http::field::content_type)
            );
            s(response);
        });
    }
private:
    RequestHandler& request_handler_;
};

template <class RequestHandler>
LoggingRequestHandler<RequestHandler> MakeHandler(RequestHandler& rh) {
    return LoggingRequestHandler<RequestHandler>(rh);
}

} // namespace logging_handler

