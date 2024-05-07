#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/bind_executor.hpp>
#include <memory>
#include <chrono>
#include "hotdog.h"
#include "result.h"
//using namespace std::literals;

namespace net = boost::asio;

// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;


  class Order : public std::enable_shared_from_this<Order> {
public:
    Order(
        net::io_context& io, int id, HotDogHandler handler, std::shared_ptr<GasCooker> gas_cooker_,
        std::shared_ptr<Sausage> sausage, std::shared_ptr<Bread> bread
    )
        : io_{io}, id_{id}, handler{std::move(handler)}, gas_cooker_(std::move(gas_cooker_)), sausage_(std::move(sausage)), bread_(std::move(bread)) { }

    // Запускает асинхронное выполнение заказа
    void Execute() {
        BakeBread();
        FrySausage();
    }

private:
    void BakeBread() {
        bread_->StartBake(*gas_cooker_, [self = shared_from_this()]() {
            self->bread_timer_.expires_from_now(Milliseconds{1000});
            self->bread_timer_.async_wait(
            net::bind_executor(self->strand_, [self = std::move(self)](sys::error_code ec) {
                self->OnBaked(ec);
            }));
        });
    }

    void OnBaked(sys::error_code ec) {

        bread_->StopBake();
            bread_baked_ = true;
        CheckReadiness();
    }

    void FrySausage() {
        sausage_->StartFry(*gas_cooker_, [self = shared_from_this()]() {
            self->sausage_timer_.expires_from_now(Milliseconds{1500});
            self->sausage_timer_.async_wait(
            net::bind_executor(self->strand_, [self = std::move(self)](sys::error_code ec) {
                self->OnFried(ec);
            }));
        });
    }

    void OnFried(sys::error_code ec) {
        sausage_->StopFry();
            sausage_fried_ = true;
        CheckReadiness();
    }

    void CheckReadiness() {
        if (delivered_) 
            return;

        // Если все компоненты гамбургера готовы, упаковываем его
        if (IsReadyToDeliver()) 
            Deliver();
        }

    void Deliver() {
        delivered_ = true;
        handler(Result{HotDog{id_, sausage_, bread_}});
    }

    bool IsReadyToDeliver(){
        return bread_baked_ && sausage_fried_ &&
               sausage_->IsCooked() && bread_->IsCooked();
    }

    int id_;
    net::io_context& io_;
    net::strand<net::io_context::executor_type> strand_{net::make_strand(io_)};
    std::shared_ptr<GasCooker> gas_cooker_;
    HotDogHandler handler;
    std::shared_ptr<Sausage> sausage_; 
    std::shared_ptr<Bread> bread_;
    bool bread_baked_ = false;
    bool sausage_fried_ = false;
    bool delivered_ = false;
    net::steady_timer bread_timer_{io_, Milliseconds{1000}};
    net::steady_timer sausage_timer_{io_, Milliseconds{1500}};
    std::atomic_int counter_{0};
};  

// Класс "Кафетерий". Готовит хот-доги
class Cafeteria: public std::enable_shared_from_this<Cafeteria> {
public:
    explicit Cafeteria(net::io_context& io)
        : io_{io} {
    }

    // Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
    // Этот метод может быть вызван из произвольного потока
    void OrderHotDog(HotDogHandler handler) {
        // TODO: Реализуйте метод самостоятельно
        // При необходимости реализуйте дополнительные классы
//	boost::system::error_code ec;
//	std::shared_ptr<Sausage> s = store_.GetSausage();
//	std::shared_ptr<Bread> b = store_.GetBread();
	const int id_ = ++nextid_;

/*	auto stopfry = [&s, this]
	{this->fry_timer.async_wait(
		net::bind_executor(this->strand_, [&s](boost::system::error_code ec)
						{s->StopFry();}
				  ));
	};*/
//	 stopfry = [&s, this]{*this->fry_timer.async_wait([&s](boost::system::error_code ec){s->StopFry();});};
//	s->StartFry(*this->gas_cooker_, stopfry);

//	stopbake = [&b, this]{*this->bake_timer.async_wait([&b](boost::system::error_code ec){b->StopBake();});};
//	b->StartBake(*this->gas_cooker_, stopbake);

//	net::post(io_, [&b, this, stopbake]{b->StartBake(*this->gas_cooker_, stopbake);});


//	if (b->IsCooked()&&s->IsCooked()) 
//	 HotDog hd(id_, s, b);

//	handler
        // 1) Выпекаем булку в течение 1 секунды, жарим сосиску в течение 1.5 секунд
        // 2) Собираем из них хот-дог
//        const int order_id = ++next_order_id_;
        std::make_shared<Order>(
            io_, id_, std::move(handler), gas_cooker_, store_.GetSausage(), store_.GetBread()
        )->Execute();


//	 handler(Result{HotDog{id_, sausage_, bread_}});
}

private:
    net::io_context& io_;
	net::strand<net::io_context::executor_type> strand_{net::make_strand(io_)};
	net::steady_timer fry_timer_{io_, std::chrono::milliseconds(1500)};
	net::steady_timer bake_timer_{io_, std::chrono::milliseconds(1000)};
	static int nextid_;

    // Используется для создания ингредиентов хот-дога
    Store store_;
    // Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
    // Используйtе её для приготовления ингредиентов хот-дога.
    // Плита создаётся с помощью make_shared, так как GasCooker унаследован от
    // enable_shared_from_this.
    std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
};
int Cafeteria::nextid_ = -1;
//take sosage
//take bread
//fry sosage
//stop fry sosage
//fry bread
//stop fry bread
