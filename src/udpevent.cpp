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

// udpevent.cpp
#include "udpevent.h"
#include "eventloop.h"
#include "event.h"
#include "wrap.h"
#include <cstring>

using namespace moon;

/**
 * @brief Constructs a new `udpevent` instance.
 *
 * Initializes the `udpevent` with the provided event loop and port number.
 * If a valid port is specified (not -1), it initializes the UDP socket by
 * calling `init_sock(port)`.
 *
 * @param base Pointer to the associated `eventloop`.
 * @param port The port number on which the UDP socket will listen. Default is
 * -1, indicating no initialization.
 */
udpevent::udpevent(eventloop* base, int port)
    : loop_(base),
      fd_(-1),
      ev_(nullptr),
      receive_cb_(nullptr),
      started_(false) {
    if (port != -1) init_sock(port);
}

/**
 * @brief Destructs the `udpevent` instance.
 *
 * Cleans up by deleting the associated `event`, closing the UDP socket file
 * descriptor, and resetting internal pointers to prevent dangling references.
 */
udpevent::~udpevent() {
    if (ev_) {
        delete ev_;
        ev_ = nullptr;
    }
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

/**
 * @brief Initializes the UDP socket and registers it with the event loop.
 *
 * Creates a non-blocking UDP socket, sets it to allow address reuse,
 * binds it to the specified port on all available interfaces,
 * and creates an associated `event` to listen for incoming UDP packets
 * with edge-triggered mode.
 *
 * @param port The port number on which the UDP socket will listen.
 */
void udpevent::init_sock(int port) {
    // 创建 UDP 套接字
    fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_ < 0) {
        perror("UDP socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置套接字为非阻塞
    setnonblock(fd_);
    setreuse(fd_);
    // 绑定到指定端口
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(fd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("UDP bind failed");
        ::close(fd_);
        exit(EXIT_FAILURE);
    }
    ev_ = new event(loop_, fd_, EPOLLIN | EPOLLET);
    ev_->setcb(std::bind(&udpevent::handle_receive, this), nullptr, nullptr);
}

/**
 * @brief Retrieves the input buffer of the UDP event.
 *
 * @return Pointer to the input `buffer` (`inbuff_`).
 */
buffer* udpevent::getinbuff() { return &inbuff_; }

eventloop* udpevent::getloop() const { return loop_; }

/**
 * @brief Sets the callback functions for receiving data and handling events.
 *
 * Assigns the provided receive callback (`rcb`) and event callback (`ecb`).
 * After setting the callbacks, it updates the event poll (EPoll) settings.
 *
 * @param rcb The callback function to be invoked when data is received.
 * @param ecb The callback function to be invoked when an event occurs (e.g.,
 * error).
 */
void udpevent::setcb(const RCallback& rcb, const Callback& ecb) {
    receive_cb_ = rcb;
    event_cb_ = ecb;
    update_ep();
}

void udpevent::setrcb(const udpevent::RCallback& rcb) {
    receive_cb_ = rcb;
    update_ep();
}

void udpevent::setecb(const udpevent::Callback& ecb) {
    event_cb_ = ecb;
    update_ep();
}

void udpevent::enable_listen() {
    if (started_) return;
    if (ev_) ev_->enable_listen();
    started_ = true;
}

void udpevent::del_listen() {
    if (!started_) return;
    if (ev_) ev_->del_listen();
    started_ = false;
}

void udpevent::update_ep() {
    if (started_) ev_->update_ep();
}

/**
 * @brief Receives data from the UDP event into a provided buffer.
 *
 * Copies up to `len` bytes of data from the input buffer (`inbuff_`) into the
 * provided `data` buffer.
 *
 * @param data Pointer to the buffer where received data will be stored.
 * @param len The maximum number of bytes to receive.
 *
 * @return The number of bytes actually received and copied into `data`.
 */
size_t udpevent::receive(char* data, size_t len) {
    return inbuff_.remove(data, len);
}

/**
 * @brief Receives a specified amount of data from the UDP event's input buffer
 * as a `std::string`.
 *
 * Copies up to `len` bytes of data from the input buffer (`inbuff_`) into a new
 * `std::string` and returns it.
 *
 * @param len The maximum number of bytes to receive.
 *
 * @return A `std::string` containing the received data.
 */
std::string udpevent::receive(size_t len) { return inbuff_.remove(len); }

/**
 * @brief Receives all available data from the UDP event's input buffer as a
 * `std::string`.
 *
 * Copies all available bytes from the input buffer (`inbuff_`) into a new
 * `std::string` and returns it.
 *
 * @return A `std::string` containing all received data.
 */
std::string udpevent::receive() {
    // return inbuff_.remove(inbuff_.readbytes());
    return inbuff_.remove();
}

/**
 * @brief Sends data to a specific address using the UDP socket.
 *
 * Sends the provided `data` string to the specified `addr` using the `sendto`
 * system call. If the send operation fails, it logs an error message.
 *
 * @param data The `std::string` containing the data to be sent.
 * @param addr The `sockaddr_in` structure containing the destination address.
 */
void udpevent::send_to(const std::string& data, const sockaddr_in& addr) {
    ssize_t n = sendto(fd_, data.c_str(), data.size(), 0,
                       (struct sockaddr*)&addr, sizeof(addr));
    if (n < 0) {
        perror("sendto failed");
    }
}

/**
 * @brief Handles incoming UDP data.
 *
 * This function is called when the UDP socket is ready to be read. It
 * continuously reads available data from the socket using `recvfrom`, appends
 * the data to the input buffer, and invokes the receive callback for each
 * received packet. If no more data is available, it breaks the loop. In case of
 * errors, it invokes the event callback.
 */
void udpevent::handle_receive() {
    while (true) {
        char buf[BUFSIZE];
        struct sockaddr_in cli_addr;
        socklen_t cli_len = sizeof(cli_addr);
        ssize_t n = recvfrom(fd_, buf, sizeof(buf), 0,
                             (struct sockaddr*)&cli_addr, &cli_len);
        if (n > 0) {
            inbuff_.append(buf, n);
            if (receive_cb_) {
                receive_cb_(cli_addr, this);
            }
        } else if (n == 0) {
            // No data
            if (event_cb_) event_cb_();
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // 没有更多数据
            } else {
                perror("recvfrom failed");
                if (event_cb_) event_cb_();
                break;
            }
        }
    }
}

void udpevent::enable_read() { ev_->enable_read(); }

void udpevent::enable_ET() { ev_->enable_ET(); }

void udpevent::disable_read() { ev_->disable_read(); }

void udpevent::disable_ET() { ev_->disable_read(); }

void udpevent::disable_cb() {
    receive_cb_ = nullptr;
    event_cb_ = nullptr;
}

udpevent::RCallback udpevent::getrcb() { return receive_cb_; }

udpevent::Callback udpevent::getecb() { return event_cb_; }
