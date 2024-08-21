#pragma once
#include <string_view>
#include <boost/json.hpp>
#include <boost/asio/strand.hpp>
#include <utility>
#include "api_handler.h"
//#include "api_handler.cpp"
#include "model.h"
#include "file_handler.h"
#include "http_server.h"
#include <filesystem>
#include "application.h"
#include "json_serializer.h"
#include <cassert>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace request_handler  {
namespace fs = std::filesystem;
namespace http = boost::beast::http;
std::string UrlDecode(const std::string&);

namespace net = boost::asio;
    
	class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
    	public:
        using Strand = net::strand<net::io_context::executor_type>;
        //using fs = std::filesystem;
        RequestHandler(std::filesystem::path root, Strand api_strand, app::Application& app)
                : root_{std::move(root)}
                , api_strand_{std::move(api_strand)}
                , app_(app){
            if (!std::filesystem::exists(root_)) {
                throw std::logic_error("path to static files not exist: " + root_.string());
            }
        }
        RequestHandler(const RequestHandler&) = delete;
        RequestHandler& operator=(const RequestHandler&) = delete;

        template <typename Body, typename Allocator, typename Send>
        void operator()
(const net::ip::tcp::endpoint&, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
		


	http_handler::ApiHandler op(app_);
		if(op.IsAPIRequest(req)){
                auto handle = [self = shared_from_this(), send, 
	handler = std::make_shared<http_handler::ApiHandler>(app_), req] {
                // Этот assert не выстрелит, так как лямбда-функция будет выполняться внутри strand
                assert(self->api_strand_.running_in_this_thread());
                send(std::move(handler->HandleApiRequest(req)));
                };
                return net::dispatch(api_strand_, handle);
            	}else {
                http_handler::FileHandler handler(root_);
                return std::visit([send](auto&& result) {
                            send(std::move(std::forward<decltype(result)>(result)));
                        }, handler.HandleFileResponse(req));
            }
        }
        ////////////функция проверки, что запрос в пределах корневого каталога
    //bool IsSubPath(std::filesystem::path, std::filesystem::path);

    private:
        const fs::path root_;
        Strand api_strand_;
        app::Application& app_;
    };
}  // namespace http_handler
