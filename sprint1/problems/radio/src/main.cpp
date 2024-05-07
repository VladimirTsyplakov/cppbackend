#include "audio.h"
#include <iostream>
#include <array>
#include <string>
#include <string_view>
#include <boost/asio.hpp>
#include <vector>
#include <algorithm>

using namespace std::literals;
namespace net = boost::asio;
using net::ip::udp;

int main(int argc, char** argv) {
std::string ch=argv[1];
if (ch=="server")
{
//Player player(ma_format_u8, 1);

        boost::asio::io_context io_context;
    Player player(ma_format_u8, 1);
    static const int port = 3333;
    static const size_t max_buffer_size = 1024;

        udp::socket socket(io_context,udp::endpoint(udp::v4(), port));

        // Запускаем сервер в цикле, чтобы можно было работать со многими клиентами
            // Создаём буфер достаточного размера, чтобы вместить датаграмму.
           // std::array<char, max_buffer_size> recv_buf;
std::array<char, 65000> recv_buf;
            udp::endpoint remote_endpoint;

            // Получаем не только данные, но и endpoint клиента
		std::cout<<"try to get mass."<<std::endl;
            auto size = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);
std::cout<<"massage got"<<std::endl;
           // std::cout << "Client said "sv << std::string_view(recv_buf.data(), size) << std::endl;

            // Отправляем ответ на полученный endpoint, игнорируя ошибку.
            // На этот раз не отправляем перевод строки: размер датаграммы будет получен автоматически.
           // boost::system::error_code ignored_error;
           // socket.send_to(boost::asio::buffer("Hello from UDP-server"sv), remote_endpoint, 0, ignored_error);
	auto fsize=size/player.GetFrameSize();
//player.PlayBuffer(boost::asio::buffer(recv_buf), fsize, 1.5s);
std::cout<<"try to play"<<std::endl;
player.PlayBuffer(recv_buf.data(), fsize, 1.5s);
std::cout<<"massage played"<<std::endl;

}
else{
try{
Recorder recorder(ma_format_u8, 1);

        boost::asio::io_context io_context;
    static const int port = 3333;

        udp::socket socket(io_context, udp::v4());
	std::string adr;
std::cout<<"adress? 127.0.0.1"<<std::endl;
getline(std::cin,adr);
        // Запускаем сервер в цикле, чтобы можно было работать со многими клиентами
            // Создаём буфер достаточного размера, чтобы вместить датаграмму.
           // std::array<char, 65000> recv_buf;

            // Получаем не только данные, но и endpoint клиента
    //        auto size = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

//            std::cout << "Client said "sv << std::string_view(recv_buf.data(), size) << std::endl;

            // Отправляем ответ на полученный endpoint, игнорируя ошибку.
            // На этот раз не отправляем перевод строки: размер датаграммы будет получен автоматически.
            boost::system::error_code ignored_error;
  //          socket.send_to(boost::asio::buffer("Hello from UDP-server"sv), remote_endpoint, 0, ignored_error);
std::cout<<"say"<<std::endl;
       auto rec_result = recorder.Record(65000, 1.5s);
Player player(ma_format_u8,1);
        boost::system::error_code ec;
        auto endpoint = udp::endpoint(net::ip::make_address(adr, ec), port);
      auto fsize=rec_result.frames*player.GetFrameSize();

//std::array<char, 65000> recv_bufs;

        socket.send_to(boost::asio::buffer(rec_result.data), endpoint);
player.PlayBuffer(rec_result.data.data(), rec_result.frames, 1.5s);
std::cout<<"try to play"<<std::endl;
//player.PlayBuffer(recv_bufs.data(), fsize, 1.5s);

	std::cout<<"massage sent to "<<adr<<std::endl;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }


}
}

// Recorder recorder(ma_format_u8, 1);
 //   Player player(ma_format_u8, 1);
 //   static const int port = 3333;
 //   static const size_t max_buffer_size = 1024;


 /*   while (true) {
        std::string str;

        std::cout << "Press Enter to record message..." << std::endl;
        std::getline(std::cin, str);

        auto rec_result = recorder.Record(65000, 1.5s);
        std::cout << "Recording done" << std::endl;

        player.PlayBuffer(rec_result.data.data(), rec_result.frames, 1.5s);
        std::cout << "Playing done" << std::endl;
    }

    return 0;
}
*/
/*
    try {
        boost::asio::io_context io_context;

        udp::socket socket(io_context, udp::endpoint(udp::v4(), port));

        // Запускаем сервер в цикле, чтобы можно было работать со многими клиентами
        for (;;) {
            // Создаём буфер достаточного размера, чтобы вместить датаграмму.
            std::array<char, max_buffer_size> recv_buf;
            udp::endpoint remote_endpoint;

            // Получаем не только данные, но и endpoint клиента
            auto size = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

            std::cout << "Client said "sv << std::string_view(recv_buf.data(), size) << std::endl;

            // Отправляем ответ на полученный endpoint, игнорируя ошибку.
            // На этот раз не отправляем перевод строки: размер датаграммы будет получен автоматически.
            boost::system::error_code ignored_error;
            socket.send_to(boost::asio::buffer("Hello from UDP-server"sv), remote_endpoint, 0, ignored_error);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }*/

