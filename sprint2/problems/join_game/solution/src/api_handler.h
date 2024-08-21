#pragma once
#ifndef GAME_SERVER_API_HANDLER_H
#define GAME_SERVER_API_HANDLER_H
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/json.hpp>
#include <string_view>
//#include "content_type.h"
#include "error_response.h"
#include "endpoint.h"
#include "make_response.h"
#include "http_server.h"
#include "tag_invoke.h"
#include "application.h"
//#include "request_handler.h"


namespace http_handler {
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace json = boost::json;
    //using namespace std::string_view_literals;
    
	struct CacheControl {
        CacheControl() = delete;
        constexpr static std::string_view NO_CACHE = "no-cache"sv;};

	std::string UrlDecode(const std::string& encoded);

    struct Api {
        Api() = delete;
        constexpr static std::string_view GET      = "GET"sv;
        constexpr static std::string_view HEAD     = "HEAD"sv;
        constexpr static std::string_view GET_HEAD = "GET, HEAD"sv;
        constexpr static std::string_view POST     = "POST"sv;};

    class ApiHandler{
    public:
        explicit ApiHandler(app::Application& app): app_(app) {}
        ApiHandler(const ApiHandler&) = delete;
        ApiHandler& operator=(const ApiHandler&) = delete;
        bool IsAPIRequest(const http_handler::StringRequest&);
		http_handler::StringResponse HandleApiRequest(const http_handler::StringRequest& req);

    private:
        http_handler::StringResponse RequestForListPlayers(const http_handler::StringRequest& req);
        static std::optional<app::Token> TryExtractToken(const http_handler::StringRequest& req);
        http_handler::StringResponse Authorize(const http_handler::StringRequest& req, const std::function<http_handler::StringResponse(const app::Player& player)>& action);
        http_handler::StringResponse RequestToJoin(const http_handler::StringRequest& req);
        http_handler::StringResponse RequestToMaps(const http_handler::StringRequest& req, std::string & decoded_target);
    private:
        app::Application& app_;
    };
}
#endif //GAME_SERVER_API_HANDLER_H
