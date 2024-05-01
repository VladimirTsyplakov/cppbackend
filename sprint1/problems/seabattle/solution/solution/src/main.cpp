#ifdef WIN32
#include <sdkddkver.h>
#endif

#include "seabattle.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <string_view>
#include <utility>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;

void PrintFieldPair(const SeabattleField& left, const SeabattleField& right) {
    auto left_pad = "  "s;
    auto delimeter = "    "s;
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
    for (size_t i = 0; i < SeabattleField::field_size; ++i) {
        std::cout << left_pad;
        left.PrintLine(std::cout, i);
        std::cout << delimeter;
        right.PrintLine(std::cout, i);
        std::cout << std::endl;
    }
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
}

template <size_t sz>
static std::optional<std::string> ReadExact(tcp::socket& socket) {
    boost::array<char, sz> buf;
    boost::system::error_code ec;

    net::read(socket, net::buffer(buf), net::transfer_exactly(sz), ec);

    if (ec) {
        return std::nullopt;
    }

    return {{buf.data(), sz}};
}

static bool WriteExact(tcp::socket& socket, std::string_view data) {
    boost::system::error_code ec;

    net::write(socket, net::buffer(data), net::transfer_exactly(data.size()), ec);

    return !ec;
}

class SeabattleAgent {
public:
    SeabattleAgent(const SeabattleField& field)
        : my_field_(field) {
    }

void ReadMove(tcp::socket& socket, bool& my_initiative){
	boost::system::error_code ec;
	std::cout<<"waiting for opponent move"<<std::endl;
	char got_moveb[2];
	size_t size = socket.read_some(net::buffer(got_moveb), ec);
	if(ec)std::cerr<<ec.what();
	std::string got_move(got_moveb);
	auto check = ParseMove(got_move);
	int temp = (int) this->my_field_.Shoot(check->second, check->first);
	std::string tempout;
	if(temp==0) {my_field_.MarkMiss(check->first, check->second); my_initiative=true; tempout="0";}
	if(temp==1) {my_field_.MarkHit(check->first, check->second); tempout="1";}
	if(temp==2) {my_field_.MarkKill(check->first, check->second);tempout="2";}
	SendResult(socket, tempout);
} //get move from opponent

auto  ReadResult(tcp::socket& socket, bool& my_initiative){
	boost::system::error_code ec;
        char got_result[2];
        socket.read_some(net::buffer(got_result), ec);
        if(ec)std::cerr<<ec.what();
	std::string got_result1(got_result);
	return got_result1;
}//get result from opponent

void SendMove(tcp::socket& socket, bool& my_initiative){
	boost::system::error_code ec;
	std::string move;
	std::cout<<"make your move (b6 for example)"<<std::endl;
	std::cin>>move;
	socket.write_some(net::buffer(move,move.size()), ec);
	if(ec)std::cout<<ec.what();
	std::string MoveResult = ReadResult(socket, my_initiative);
	std::string fig = "1";
	auto a = ParseMove(move.data());
	if(MoveResult<fig){other_field_.MarkMiss(a->second, a->first); my_initiative = false; std::cout<<"missed"<<std::endl;}
	if(MoveResult==fig){other_field_.MarkHit(a->second, a->first); std::cout<<"hit"<<std::endl;}
	if(MoveResult>fig){other_field_.MarkKill(a->second, a->first); std::cout<<"killed"<<std::endl;}
}//send my move to opponent

void SendResult(tcp::socket& socket, std::string temp){
	boost::system::error_code ec;
	socket.write_some(net::buffer(temp,temp.size()), ec);
}//send result to opponent

    void StartGame(tcp::socket& socket, bool my_initiative) {
        // TODO: реализуйте самостоятельно
	while (!IsGameEnded()) {
		
		this->PrintFields();

			if(my_initiative) {
				this->SendMove(socket, my_initiative);
					  }
			else{
				this->ReadMove(socket, my_initiative);
			    }
			
				}
	
    }

private:
    static std::optional<std::pair<int, int>> ParseMove(const std::string_view& sv) {
        if (sv.size() != 2) return std::nullopt;

        int p1 = sv[0] - 'A', p2 = sv[1] - '1';

        if (p1 < 0 || p1 > 8) return std::nullopt;
        if (p2 < 0 || p2 > 8) return std::nullopt;

        return {{p1, p2}};
    }

    static std::string MoveToString(std::pair<int, int> move) {
        char buff[] = {static_cast<char>(move.first) + 'A', static_cast<char>(move.second) + '1'};
        return {buff, 2};
    }

    void PrintFields() const {
        PrintFieldPair(my_field_, other_field_);
    }

    bool IsGameEnded() const {
        return my_field_.IsLoser() || other_field_.IsLoser();
    }

    // TODO: добавьте методы по вашему желанию cleanmonitor na primer

private:
    SeabattleField my_field_;
    SeabattleField other_field_;
};

void StartServer(const SeabattleField& field, unsigned short port) {
    SeabattleAgent agent(field);

    // TODO: реализуйте самостоятельно
	boost::system::error_code ec;//error
	net::io_context iocontext;//stream in/out
	tcp::socket socket{iocontext};//socket
	tcp::acceptor acceptor(iocontext, tcp::endpoint(tcp::v4(),port));//to accept connection from...
	acceptor.accept(socket, ec);//accept connection

    agent.StartGame(socket, false);
}

void StartClient(const SeabattleField& field, const std::string& ip_str, unsigned short port) {
    SeabattleAgent agent(field);

    // TODO: реализуйте самостоятельно
	boost::system::error_code ec;
	net::io_context iocontext;
	auto endpoint = tcp::endpoint(net::ip::make_address(ip_str, ec), port);
	tcp::socket socket{iocontext};
	socket.connect(endpoint, ec);

    agent.StartGame(socket, true);
}

int main(int argc, const char** argv) {
    if (argc != 3 && argc != 4) {
        std::cout << "Usage: program <seed> [<ip>] <port>" << std::endl;
        return 1;
    }

    std::mt19937 engine(std::stoi(argv[1]));
    SeabattleField fieldL = SeabattleField::GetRandomField(engine);

    if (argc == 3) {
        StartServer(fieldL, std::stoi(argv[2]));
    } else if (argc == 4) {
        StartClient(fieldL, argv[2], std::stoi(argv[3]));
    }
}
