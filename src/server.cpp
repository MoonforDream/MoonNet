#include "server.h"
#include "event.h"
#include "udpevent.h"
#include "signalevent.h"
#include "timerevent.h"


using namespace moon;



server::server(int port)
:port_(port),pool_(&base_),acceptor_(port,&base_){
    if(port>0) enable_tcp(port);
}


server::~server(){
    stop();
    if(tcp_enable_) acceptor_.stop();
    std::lock_guard<std::mutex> lock(events_mutex_);
    for(auto &ev: events_){
        ev->close();
        delete ev;
    }
    events_.clear();
}


void server::start(){
    base_.loop();
}


void server::stop() {
    pool_.stop();
    base_.loopbreak();
}


void server::init_pool(int timeout){
    pool_.create_pool(timeout);
}



void server::init_pool(int tnum,int timeout){
    pool_.create_pool(tnum,timeout);
}


void server::init_pool_noadjust(int tnum,int timeout){
    pool_.create_pool_noadjust(tnum,timeout);
}


eventloop* server::getloop(){
    return &base_;
}


void server::enable_tcp(int port){
    if(tcp_enable_) return;
    port_=port;
    acceptor_.init_sock(port);
    acceptor_.setcb(std::bind(&server::acceptcb_,this,std::placeholders::_1));
    acceptor_.listen();
    tcp_enable_=true;
}




void server::enable_tcp_accept(){
    acceptor_.listen();
}



void server::disable_tcp_accept(){
    acceptor_.stop();
}



eventloop* server::dispatch(){
    return pool_.ev_dispatch();
}


void server::set_tcpcb(const RCallback &rcb,const Callback &wcb,const Callback &ecb){
    readcb_=rcb;
    writecb_=wcb;
    eventcb_=ecb;
}



void server::add_ev(event *ev){
    ev->enable_listen();
    events_.emplace_back(ev);
}



void server::del_ev(event *ev){
    ev->del_listen();
    handle_close(ev);
}


void server::mod_ev(event *ev){
    ev->update_ep();
}


void server::add_bev(bfevent *bev){
    bev->enable_listen();
    events_.emplace_back(bev);
}


void server::del_bev(bfevent *bev){
    bev->del_listen();
    handle_close(bev);
}


void server::mod_bev(bfevent *bev){
    bev->update_ep();
}


void server::add_udpev(udpevent *uev){
    udpevent::Callback cb=uev->getecb();
    uev->setecb([&](){
        if(cb) cb();
        handle_close(uev);
    });
    uev->start();
    events_.emplace_back(uev);
}



udpevent* server::add_udpev(int port,const UCallback &rcb,const Callback &ecb){
    udpevent *uev=new udpevent(pool_.ev_dispatch(),port);
    uev->setcb(rcb,[&](){
        if(ecb) ecb();
        handle_close(uev);
    });
    uev->start();
    events_.emplace_back(uev);
    return uev;
}


void server::del_udpev(udpevent *uev){
    uev->stop();
    handle_close(uev);
}


void server::mod_udpev(udpevent *uev){
    uev->update_ep();
}



void server::add_sev(signalevent *sev) {
    sev->enable_listen();
    events_.emplace_back(sev);
}


signalevent* server::add_sev(int signo, const moon::server::SCallback &cb) {
    signalevent* sigev=new signalevent(&base_);
    sigev->add_signal(signo);
    sigev->setcb(cb);
    sigev->enable_listen();
    events_.emplace_back(sigev);
    return sigev;
}



signalevent* server::add_sev(const std::vector<int>& signals, const moon::server::SCallback &cb) {
    signalevent* sigev=new signalevent(&base_);
    sigev->add_signal(signals);
    sigev->setcb(cb);
    sigev->enable_listen();
    events_.emplace_back(sigev);
    return sigev;
}


void server::del_sev(moon::signalevent *sev) {
    sev->del_listen();
    handle_close(sev);
}



timerevent* server::add_timeev(int timeout_ms, bool periodic, const Callback &cb) {
    timerevent* tev=new timerevent(dispatch(),timeout_ms,periodic);
    tev->setcb(cb);
    tev->start();
    events_.emplace_back(tev);
    return tev;
}


void server::add_timeev(timerevent *tev) {
    tev->start();
    events_.emplace_back(tev);
}


void server::del_timeev(timerevent *tev) {
    tev->stop();
    handle_close(tev);
}
