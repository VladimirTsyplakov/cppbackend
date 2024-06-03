#include "sdk.h"
//

#define BOOST_BEAST_USE_STD_STRING_VIEW
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <thread>
#include <boost/asio/signal_set.hpp>
#include <vector>

#include "http_server.h"
#include "json_loader.h"
#include "request_handler.h"
#include <boost/asio/steady_timer.hpp>
#include "model.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
namespace beast = boost::beast;
namespace http = boost::beast::http;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

namespace {

// Создаёт StringResponse с заданными параметрами
/*StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                  bool keep_alive,
                                  std::string_view content_type = ContentType::TEXT_HTML) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}
StringResponse HandleRequest(StringRequest&& req) {
    // Подставьте сюда код из синхронной версии HTTP-сервера
    const auto text_response = [&req](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, req.version(), req.keep_alive());
    };
        std::string greet;
        http::status stat = http::status::ok;
        std::string_view ques = req.method_string();
        if(ques == "GET"){
                std::string_view tar = req.target();
        std::transform(map.begin(), map.end() , [](
    // Здесь можно обработать запрос и сформировать ответ, но пока всегда отвечаем: Hello
//              greet = "<strong>Hello</strong>";
                greet = "Hello, ";
                tar.remove_prefix(1);
//              greet.insert(13, ", ");
//              greet.insert(15, tar);
                greet.insert(7, tar);
                         }
        if(ques != "GET" && ques != "HEAD"){
//      greet = "<strong>Invalid method</strong";  
        greet = "Invalid method";
        stat = http::status::method_not_allowed;}

        return text_response(stat, greet);
} 
*/
// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: game_server <game-config-json>"sv << std::endl;
        return EXIT_FAILURE;
    }
    const unsigned num_threads = std::thread::hardware_concurrency();
///    net::io_context ioc(num_threads);


    try {
        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(argv[1]);

        // 2. Инициализируем io_context
//        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
	    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
        if (!ec) {
            ioc.stop();
        }
    });
        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        request_handler::RequestHandler handler{game};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
	const auto address = net::ip::make_address("0.0.0.0");
	constexpr net::ip::port_type port = 8080;

        http_server::ServeHttp(ioc, {address, port}, [&handler](auto&& req, auto&& send) {
            handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        });

        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        std::cout << "Server has started..."sv << std::endl;

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;}
std::cout << "Shutting down"sv << std::endl;
}
