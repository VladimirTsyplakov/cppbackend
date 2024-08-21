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
#include "logging_request_handler.h"
#include "logger.h"

using namespace std::literals;
//using namespace json_logger;
namespace net = boost::asio;
namespace sys = boost::system;
namespace beast = boost::beast;
namespace http = boost::beast::http;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
//    static JsonLogger& GetInstance();
using StringResponse = http::response<http::string_body>;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) 
	{
        workers.emplace_back(fn);
	}
    fn();
}
}  // namespace

int main(int argc, const char* argv[]) {
   
if (argc != 2 && argc != 3) {
        std::cerr << "Usage: game_server <game-config-json> root_catalogue_path"sv << std::endl;
        return EXIT_FAILURE;
    }

    const unsigned num_threads = std::thread::hardware_concurrency();
///    net::io_context ioc(num_threads);


    try {
        // 1. Загружаем карту из файла и построить модель игры
        //model::Game game = json_loader::LoadGame(argv[1]);
	//	определляем корневой каталог

	    std::filesystem::path conf_file = argv[1];
	    std::filesystem::path base_cat_path = argv[2];
	            app::Application application(conf_file);    
	//auto logger = json_logger::JsonLogger::GetInstance();
	//json_logger::JsonLogger::GetInstance().LogServerStarted({address, port});;
        // 2. Инициализируем io_context
//        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);
        auto api_strand = net::make_strand(ioc);
        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
	    net::signal_set signals(ioc, SIGINT, SIGTERM);

    	signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
        if (!ec) {ioc.stop();}});
        
	// 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        auto handler = std::make_shared<request_handler::RequestHandler>(
                base_cat_path, api_strand, application);
	logging_handler::LoggingRequestHandler l_handler{(*handler)};

// 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
	const auto address = net::ip::make_address("0.0.0.0");
	constexpr net::ip::port_type port = 8080;
                json_logger::JsonLogger::GetInstance().LogServerStarted({address, port});

	http_server::ServeHttp(ioc,{address, port}, l_handler);

        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
//        std::cout << "Server has started..."sv << std::endl;
	//logger.LogServerStarted({address, port});
        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
	//logger.LogServerNormalFinish();
	json_logger::JsonLogger::GetInstance().LogServerNormalFinish();
        } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
	//json_logger::JsonLogger::GetInstance().LogServerErrorFinish(ex);
	json_logger::JsonLogger::GetInstance().LogServerErrorFinish(ex);
        return EXIT_FAILURE;}
	//std::cout << "Shutting down"sv << std::endl;
}

