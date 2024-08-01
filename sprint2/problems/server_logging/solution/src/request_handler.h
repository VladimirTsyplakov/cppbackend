#pragma once

#include <boost/json.hpp>

#include "json_serializer.h"
#include "http_server.h"
#include "model.h"
#include <filesystem>
#include <cassert>
#include <map>

namespace net = boost::asio;

// Для синхронного вывода в Boost.Asio могут использовать любой тип, удовлетворяющий требованиям,
// описанным в документе:
// https://www.boost.org/doc/libs/1_78_0/doc/html/boost_asio/reference/SyncWriteStream.html
class SyncWriteOStreamAdapter {
public:
    explicit SyncWriteOStreamAdapter(std::ostream& os)
        : os_{os} {
    }

    template <typename ConstBufferSequence>
    size_t write_some(const ConstBufferSequence& cbs, std::error_code& ec) {
        const size_t total_size = net::buffer_size(cbs);
        if (total_size == 0) {
            ec = {};
            return 0;
        }
        size_t bytes_written = 0;
        for (const auto& cb : cbs) {
            const size_t size = cb.size();
            const char* const data = reinterpret_cast<const char*>(cb.data());
            if (size > 0) {
                if (!os_.write(reinterpret_cast<const char*>(data), size)) {
                    ec = make_error_code(boost::system::errc::io_error);
                    return bytes_written;
                }
                bytes_written += size;
            }
        }
        ec = {};
        return bytes_written;
    }

    template <typename ConstBufferSequence>
    size_t write_some(const ConstBufferSequence& cbs) {
 //       sys::error_code ec;
    std::error_code ec;
        const size_t bytes_written = write_some(cbs, ec);
        if (ec) {
            throw std::runtime_error("Failed to write");
        }
        return bytes_written;
    }

private:
    std::ostream& os_;

};

namespace request_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace sys = boost::system;
using namespace std::literals;
using namespace model;

using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

// Создаёт StringResponse с заданными параметрами
StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                  bool keep_alive,
                                  std::string_view content_type = "text/html");

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, std::string cat_path)
        : game_{game}, base_cat_path_{cat_path} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;
///////////// функция декодер строки запроса
    const std::filesystem::path Decode(const std::string_view tar) {
    namespace fs = std::filesystem;
    const fs::path abs_path = fs::weakly_canonical(tar);
    return abs_path;
    }
////////////функция проверки, что запрос в пределах корневого каталога
    bool IsSubPath(std::filesystem::path path, std::filesystem::path base) {
    // Приводим оба пути к каноничному виду (без . и ..)
    namespace fs = std::filesystem;
    path = fs::weakly_canonical(path);
    base = fs::weakly_canonical(base);

    // Проверяем, что все компоненты base содержатся внутри path
    for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
        if (p == path.end() || *p != *b) {
            return false;
        }
    }
    return true;
}
     std::string extention_to_content(const std::string ext)
    { const auto it = type_map_.find(ext);
        return (it != type_map_.end()) ? it->second : "application/octet-stream";}
//обрабатываем строку
    std::string type_file_content(const std::string path)
    {
    //получаем субстроку
    std::string lowpath = path.substr(path.find_last_of('.'));
    //выравниваем регистр
    std::transform(lowpath.begin(), lowpath.end(), lowpath.begin(), [](unsigned char c){return std::tolower(c);});
    //возвращаем contenttype из списка возможных или application/octet-stream
    return extention_to_content(lowpath);
    }

    template <typename Body, typename Allocator, typename Send>
    void operator()(const net::ip::tcp::endpoint&, http::request<Body, http::basic_fields<Allocator>>&& request, Send&& send) {
        using namespace json_serializer;

        auto json_response = [&request](http::status status, 
                    json::value value, 
                    std::string_view cont_type = "application/json"sv) {
            return MakeStringResponse(status, json::serialize(value),
                                      request.version(), request.keep_alive(), cont_type);
        };

        auto target = request.target();
        std::string_view endpoint = "/api/v1/maps";
//////////
//    std::filesystem::path root_path = "~/cpp-backend/sprint2/problems/static_content/solution/static";
/*    fs::path base_path{"/this/is/some/path/"s};
    fs::path rel_path{"../../another/path"s};
    fs::path abs_path = fs::weakly_canonical(base_path / rel_path);*/
/*    assert(IsSubPath(Decode(target), root_path));
    assert(IsSubPath(Decode(target), root_path));
    assert(!IsSubPath(Decode(target), root_path));*/
///////////

//если запрос начинается с АРI ! нужны карты
if(target.starts_with("/api/"sv)){
    //если запрос на все карты"/api/v1/maps"
        if (target == endpoint){send(json_response(http::status::ok, Serialize(game_.GetMaps())));}
        //если запрос на какую!то конкретную карту /api/v1/maps/тратата
        else if (target.starts_with(endpoint) && !target.ends_with("/"sv)) {
		std::string_view id = target.substr(endpoint.size() + 1);
                auto map_id = Map::Id{std::string{id}};
                const auto* map_ptr = game_.FindMap(map_id);
            //если карты с таким ИД не существует
                    if (map_ptr == nullptr)
                    send(json_response(http::status::not_found,
                        SerializeError("mapNotFound", "Map not found")));
                    else
                    {send(json_response(http::status::ok, Serialize(*map_ptr)));}}

        //если запрос начинается на /api/ заканчивается /
        else if (target.starts_with("/api/"sv)) {
                send(json_response(http::status::bad_request,
                SerializeError("badRequest", "Bad request")));}
}
//если запрос на файл или каталог
else{
    auto dec_target = Decode(target);
    std::filesystem::path path = base_cat_path_;
	path += dec_target;
/*	std::string test{base_cat_path_};
	std::string test2{dec_target};
	std::string test3 = test+test2;
std::filesystem::path path = std::filesystem::weakly_canonical(test3);*/
        //если на каталог
    if (dec_target.string().ends_with("/")) {
        //если каталог валидный

        if (IsSubPath(path, base_cat_path_)) {
            //вернуть index html

            http::response<http::file_body> res;
            res.version(11);  // HTTP/1.1
            res.result(http::status::ok);
            res.insert(http::field::content_type, "text/html"sv);
	    http::file_body::value_type file;

            path += std::filesystem::weakly_canonical(Decode("index.html"));
            if (sys::error_code ec; 
		file.open(
		path.string().c_str(),
                beast::file_mode::read, ec), ec) {
                std::cout << "Failed to open file "<< path.string().c_str() << ec.what()<< std::endl;
               // return EXIT_FAILURE; 
            }

            res.body() = std::move(file);
            // Метод prepare_payload заполняет заголовки Content-Length и Transfer-Encoding
            // в зависимости от свойств тела сообщения
            res.prepare_payload();
		send(res);
		//печатаем отчет
		http::file_body::value_type file2;
                if(sys::error_code ec; file2.open(path.string().c_str(), beast::file_mode::read, ec), ec) {
                std::cout << "Failed to open file "<< path.string().c_str() << ec.what()<< std::endl;}
		http::response<http::file_body> res2;
                res2.body() = std::move(file2);
		SyncWriteOStreamAdapter adapter{ std::cout };
		boost::beast::http::write(adapter, res2);
        }
        //послать  400
        else {
            send(json_response(http::status::bad_request,
                SerializeError("badRequest", "Bad request"), "text/plain"));
        }
    }

        //если на файл
        else {
            //если на файл в static
            if (IsSubPath(path, base_cat_path_)) {
                // он существует?
                using namespace http;
//                auto dec_target = Decode(target);
//                std::filesystem::path file_path = std::filesystem::weakly_canonical(base_cat_path_ += dec_target);

                file_body::value_type file;

                if (boost::system::error_code ec; file.open(path.string().c_str(), 
							beast::file_mode::read, ec), ec) {
                    send(json_response(http::status::not_found, SerializeError("The file notFound", "Not Found"), "text/plain"));
                }
                else//выдаем файл
                {
                    response<http::file_body> res;
                    res.version(11);  // HTTP/1.1
                    res.result(status::ok);
                    res.insert(field::content_type, type_file_content(path));
                    //file_body::value_type file;
                    res.body() = std::move(file);
                    // Метод prepare_payload заполняет заголовки Content-Length и Transfer-Encoding
                    // в зависимости от свойств тела сообщения
                    res.prepare_payload();
                    //SyncWriteOStreamAdapter adapter{ std::cout };
                    //boost::beast::http::write(adapter, res);
		    send(res);
		//печатаем отчет
		http::file_body::value_type file2;
                if(sys::error_code ec; file2.open(path.string().c_str(), beast::file_mode::read, ec), ec) {
                std::cout << "Failed to open file "<< path.string().c_str() << ec.what()<< std::endl;}
                http::response<http::file_body> res2;
                res2.body() = std::move(file2);

            SyncWriteOStreamAdapter adapter{ std::cout };
            boost::beast::http::write(adapter, res2);

///////////////////////////////////////////////////////////////////////////////////////////////////////
                }//выдали файл
            }//на файл в static 
        //послать
            else {
                send(json_response(http::status::bad_request,
                    SerializeError("badRequest", "Bad request"), "text/plain"));
            }
         }//на файл
    }//на файл или каталог
}//operator
private:
    Game& game_;
    std::filesystem::path base_cat_path_;
    const std::unordered_map<std::string, std::string> type_map_=
    {
     {".htm", "text/html"},
     {".html", "text/html"},
     {".css", "text/css"},
     {".txt:", "text/plain"},
     {".js", "text/javascript"},
     {".json", "application/json"},
     {".xml", "application/xml"},
     {".png", "image/png"},
     {".jpg", "image/jpeg"},
     {".jpe", "image/jpeg"},
     {".jpeg", "image/jpeg"},
     {".gif", "image/gif"},
     {".bmp", "image/bmp"},
     {".ico", "image/vnd.microsoft.icon"},
     {".tiff", "image/tiff"},
     {".tif", "image/tiff"},
     {".svg", "image/svg+xml"},
     {".svgz", "image/svg+xml"},
     {".mp3", "audio/mpeg"}
    };
//operator()
};//class
}  // namespace request_handler
