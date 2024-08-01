#include "logger.h"
//#define BOOST_LOG_DYN_LINK 1
namespace json_logger {

namespace logging = boost::log;
namespace json = boost::json;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace sys = boost::system;
namespace net = boost::asio;
namespace http = boost::beast::http;

using tcp = net::ip::tcp;

//std::mutex mutex;

void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    json::value data = *rec[additional_data];
    auto ts = *rec[timestamp];
    auto message = *rec[expr::smessage];

    json::value result = {
        {"timestamp", to_iso_extended_string(ts)},
        {"data", data},
        {"message", message}
    };

    strm << result;
}

JsonLogger& JsonLogger::GetInstance() {
   // std::lock_guard<std::mutex> lock(mutex);
    static JsonLogger logger;
    return logger;
}

void JsonLogger::LogJson(std::string_view message, const json::value& data) {
   // std::lock_guard g{mutex};
    BOOST_LOG_TRIVIAL(trace) << logging::add_value(additional_data, data) << message;
}

void JsonLogger::LogError(std::string_view where, const sys::error_code& ec) {
    json::value data = {
        {"code", ec.value()},
        {"text", ec.message()},
        {"where", where}
    };
    LogJson("error", data);
}

void JsonLogger::LogServerStarted(const tcp::endpoint& ep) {
    json::value data = {
        {"port", ep.port()},
        {"address", ep.address().to_string()}
    };
    LogJson("server started", data);
}

void JsonLogger::LogServerNormalFinish() {
    json::value data = {
        {"code", 0}
    };
    LogJson("server exited", data);
}

void JsonLogger::LogServerErrorFinish(const std::exception& ec) {
    json::value data = {
        {"code", "EXIT_FAILURE"},
        {"exception", ec.what()}
    };
    LogJson("server exited", data);
}

//template <typename Body, typename Allocator>
//void JsonLogger::LogRequest(const boost::asio::ip::tcp::endpoint &endpoint, 
//	const http::request<Body, http::basic_fields<Allocator>> &req) {
//    json::value data = {
//        {"ip", endpoint.address().to_string()},
//        {"URI", req.target()},
//        {"method", req.method_string()}
//    };
//    LogJson("request received", data);
//}

void JsonLogger::LogResponse(std::chrono::steady_clock::duration dur, unsigned int code, 
					std::string_view content_type) {
    using namespace std::chrono;
    json::value data = {
        {"response_time", duration_cast<milliseconds>(dur).count()},
        {"code", code},
        {"content_type", content_type}
    };
    LogJson("response sent", data);
}

JsonLogger::JsonLogger() {
    logging::add_common_attributes();
    logging::add_console_log(
        std::cout,
        keywords::format = &MyFormatter,
        keywords::auto_flush = true
    );
}
/*JsonLogger::JsonLogger(JsonLogger& op) {
    logging::add_common_attributes();
    logging::add_console_log(
        std::cout,
        keywords::format = &MyFormatter,
        keywords::auto_flush = true);
	mutex = new std::mutex;
}*/



} // namespace json_logger
