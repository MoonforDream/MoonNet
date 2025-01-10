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

#ifndef _RINGBUFF_H_
#define _RINGBUFF_H_

#include <sys/types.h>
#include <atomic>
#include <memory>
#include <vector>
#include <list>

namespace moon {

    // 无锁环形缓冲区 RingBuffer
    // lock-free ringbuffer
    template <class T>
    class ringbuff {
    public:
        ringbuff(size_t size = 1024)
            : head_(0),
              tail_(0),
              capacity_(adj_size(size)),
              buffer_(new T[capacity_]) {}
        //        ~ringbuff()=default;
        ~ringbuff() { delete[] buffer_; }
        /** push function **/
        bool push(const T& item);
        // push with move
        bool push_move(T&& item);
        /** pop function **/
        bool pop(T& item);
        // pop with move
        bool pop_move(T& item);

        /** performance function **/
        size_t capacity() const;
        size_t size() const;
        bool empty();
        bool full();
        void swap(ringbuff& other) noexcept;

        /** swap_to function **/
        void swap_to_list(std::list<T>& list_);
        void swap_to_vector(std::vector<T>& vec_);
        std::list<T> swap_to_list();
        std::vector<T> swap_to_vector();

    private:
        bool is_powtwo(size_t n) const;
        size_t next_powtwo(size_t n) const;
        size_t adj_size(size_t size) const;

    private:
        // avoid pseudo shareing
        std::atomic<size_t> head_ alignas(64);
        std::atomic<size_t> tail_ alignas(64);
        //        std::unique_ptr<T[]> buffer_;
        size_t capacity_;
        T* buffer_;
    };

}  // namespace moon

#include "ringbuff.tpp"

#endif  // !_RINGBUFF_H_
