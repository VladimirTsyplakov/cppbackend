#pragma once
#ifndef GAME_SERVER_FILE_HANDLER_H
#define GAME_SERVER_FILE_HANDLER_H
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/beast/http.hpp>
#include <boost/json.hpp>
#include <string_view>
#include <variant>
#include <filesystem>
#include "content_type.h"
#include "endpoint.h"
#include "error_response.h"
#include "make_response.h"
//#include "request_handler.h"
#include "api_handler.h"

namespace http_handler {
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace json = boost::json;
    namespace fs = std::filesystem;
    namespace sys = boost::system;
    using namespace std::literals;

//std::string UrlDecode(const std::string& encoded);
    
	class FileHandler {
    public:
        explicit FileHandler(fs::path path): root_path_(std::move(path)) {}

        FileHandler(const FileHandler&) = delete;
        FileHandler& operator=(const FileHandler&) = delete;
        bool IsSubPath(std::filesystem::path, std::filesystem::path);
        VariantResponse HandleFileResponse(const StringRequest& req);

    private:
        const fs::path root_path_;
    };
}

#endif //GAME_SERVER_FILE_HANDLER_H
