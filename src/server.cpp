/* BSD 3-Clause License

Copyright (c) 2024, MoonforDream

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Author: MoonforDream

*/

#include "server.h"
#include "event.h"
#include "udpevent.h"
#include "signalevent.h"
#include "timerevent.h"

using namespace moon;

/**
 * @brief Constructs a new server instance.
 *
 * Initializes the server with an optional port number. If dispatching is enabled, it initializes the adjustment mechanism.
 *
 * @param port The port number for TCP service. Default is -1, which means TCP service is not enabled initially.
 */
server::server(int port) : pool_(&base_),acceptor_(port, &base_),port_(port){
    if (port > 0) enable_tcp(port);
}

server::~server() {
    stop();
    if (tcp_enable_) acceptor_.stop();
    std::lock_guard<std::mutex> lock(events_mutex_);
    for (auto &ev : events_) {
        ev->close();
        delete ev;
    }
    events_.clear();
}

void server::start() { base_.loop(); }

void server::stop() {
    pool_.stop();
    base_.loopbreak();
}

void server::init_pool(int timeout) { pool_.create_pool(timeout); }

void server::init_pool(int tnum, int timeout) {
    pool_.create_pool(tnum, timeout);
}

void server::init_pool_noadjust(int tnum, int timeout) {
    pool_.create_pool_noadjust(tnum, timeout);
}

eventloop *server::getloop() { return &base_; }

/**
 * @brief Enables TCP support on a specified port.
 *
 * Initializes the TCP acceptor on the given port, sets the callback function
 * for accepting new connections, and starts listening for incoming TCP
 * connections. If TCP is already enabled, the function returns immediately
 * without performing any actions.
 *
 * @param port The port number on which to enable TCP listening.
 */
void server::enable_tcp(int port) {
    if (tcp_enable_) return;
    port_ = port;
    acceptor_.init_sock(port);
    acceptor_.setcb(std::bind(&server::acceptcb_, this, std::placeholders::_1));
    acceptor_.listen();
    tcp_enable_ = true;
}

void server::enable_tcp_accept() { acceptor_.listen(); }

void server::disable_tcp_accept() { acceptor_.stop(); }

/**
 * @brief Dispatches events to an appropriate event loop.
 *
 * Selects an event loop from the pool based on the current dispatching
 * strategy. If dispatching is enabled, it selects the loop with the minimum
 * load. Otherwise, it uses a round-robin approach to select the next loop.
 *
 * @return Pointer to the selected `eventloop` instance.
 */
eventloop *server::dispatch() { return pool_.ev_dispatch(); }

void server::set_tcpcb(const RCallback &rcb, const Callback &wcb,
                       const Callback &ecb) {
    readcb_ = rcb;
    writecb_ = wcb;
    eventcb_ = ecb;
}

/**
 * @brief Adds an event to the server's event list.
 *
 * Adds a new event (`ev`) to the server's event list. If the event is a UDP
 * event, it sets up an additional callback to handle the closure of the event
 * after it is processed. The event is then enabled to listen for relevant
 * events and stored in the event list.
 *
 * @param ev Pointer to the `base_event` to be added.
 */
void server::addev(base_event *ev) {
    if (auto uev = dynamic_cast<udpevent *>(ev)) {
        udpevent::Callback cb = uev->getecb();
        uev->setecb([&]() {
            if (cb) cb();
            handle_close(uev);
        });
    }
    ev->enable_listen();
    events_.emplace_back(ev);
}

/**
 * @brief Modifies an existing event in the server's event list.
 *
 * Updates the event's event poll (EPoll) settings to reflect any changes.
 *
 * @param ev Pointer to the `base_event` to be modified.
 */
void server::modev(base_event *ev) { ev->update_ep(); }

/**
 * @brief Deletes an event from the server's event list.
 *
 * Removes an event (`ev`) from the server's event list, stops it from
 * listening, and handles any necessary cleanup.
 *
 * @param ev Pointer to the `base_event` to be deleted.
 */
void server::delev(base_event *ev) {
    ev->del_listen();
    handle_close(ev);
}

/**
 * @brief Adds a UDP event to the server.
 *
 * Creates a new UDP event on the specified port with the provided read and
 * error callbacks, enables it to listen for incoming UDP packets, and adds it
 * to the server's event list. If an error occurs during event creation, the
 * function ensures automatic cleanup.
 *
 * @param port The port number on which to listen for UDP packets.
 * @param rcb The callback function to be invoked when a read event occurs on
 * the UDP socket.
 * @param ecb The callback function to be invoked when an error event occurs on
 * the UDP socket.
 *
 * @return Pointer to the newly created `udpevent` instance.
 */
udpevent *server::add_udpev(int port, const UCallback &rcb,
                            const Callback &ecb) {
    udpevent *uev = new udpevent(pool_.ev_dispatch(), port);
    uev->setcb(rcb, [&]() {
        if (ecb) ecb();
        handle_close(uev);
    });
    uev->enable_listen();
    events_.emplace_back(uev);
    return uev;
}

/**
 * @brief Adds a signal event to the server for a specific signal.
 *
 * Creates a new signal event for the specified signal number (`signo`),
 * sets its callback function, enables it to listen for the signal,
 * and adds it to the server's event list.
 *
 * @param signo The signal number to listen for (e.g., `SIGINT`, `SIGTERM`).
 * @param cb The callback function to be invoked when the specified signal is
 * received.
 *
 * @return Pointer to the newly created `signalevent` instance.
 */
signalevent *server::add_sev(int signo, const moon::server::SCallback &cb) {
    signalevent *sigev = new signalevent(&base_);
    sigev->add_signal(signo);
    sigev->setcb(cb);
    sigev->enable_listen();
    events_.emplace_back(sigev);
    return sigev;
}

/**
 * @brief Adds a signal event to the server for multiple signals.
 *
 * Creates a new signal event that listens for any of the specified signals in
 * the `signals` vector, sets its callback function, enables it to listen for
 * the signals, and adds it to the server's event list.
 *
 * @param signals A vector of signal numbers to listen for (e.g., `{SIGINT,
 * SIGTERM}`).
 * @param cb The callback function to be invoked when any of the specified
 * signals are received.
 *
 * @return Pointer to the newly created `signalevent` instance.
 */
signalevent *server::add_sev(const std::vector<int> &signals,
                             const moon::server::SCallback &cb) {
    signalevent *sigev = new signalevent(&base_);
    sigev->add_signal(signals);
    sigev->setcb(cb);
    sigev->enable_listen();
    events_.emplace_back(sigev);
    return sigev;
}

/**
 * @brief Adds a timer event to the server.
 *
 * Creates a new timer event that triggers after a specified timeout,
 * optionally repeating periodically. It sets the callback function to be
 * invoked when the timer fires, enables the timer to start listening, and adds
 * it to the server's event list.
 *
 * @param timeout_ms The timeout duration in milliseconds after which the timer
 * event is triggered.
 * @param periodic A boolean flag indicating whether the timer should trigger
 * periodically.
 * @param cb The callback function to be invoked when the timer event fires.
 *
 * @return Pointer to the newly created `timerevent` instance.
 */
timerevent *server::add_timeev(int timeout_ms, bool periodic,
                               const Callback &cb) {
    timerevent *tev = new timerevent(dispatch(), timeout_ms, periodic);
    tev->setcb(cb);
    tev->enable_listen();
    events_.emplace_back(tev);
    return tev;
}

/** v1.0.0 function
 * unused function now **/
/* void server::del_udpev(udpevent *uev){
    uev->del_listen();
    handle_close(uev);
}


void server::mod_udpev(udpevent *uev){
    uev->update_ep();
}



void server::add_sev(signalevent *sev) {
    sev->enable_listen();
    events_.emplace_back(sev);
}

void server::del_sev(moon::signalevent *sev) {
    sev->del_listen();
    handle_close(sev);
}


void server::add_timeev(timerevent *tev) {
    tev->enable_listen();
    events_.emplace_back(tev);
}


void server::del_timeev(timerevent *tev) {
    tev->del_listen();
    handle_close(tev);
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
    uev->enable_listen();
    events_.emplace_back(uev);
}
 */
