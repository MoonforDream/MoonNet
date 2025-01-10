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

#ifndef _LFTHREAD_H_
#define _LFTHREAD_H_

#include <functional>
#include <thread>
#include <list>
#include <vector>
#include "ringbuff.h"

namespace moon {

    class lfthread {
    public:
        using task = std::function<void()>;
        lfthread(size_t size)
            : buffer_(size),
              shutdown_(false),
              t_(std::thread(&lfthread::t_task, this)) {}
        ~lfthread() { t_shutdown(); }
        bool enqueue_task(task _task);
        bool enqueue_task_move(task&& _task);
        void t_shutdown();
        int getload() const;
        void swap_to_ringbuff(ringbuff<task>& rb_);
        void swap_to_list(std::list<task>& list_);
        void swap_to_vector(std::vector<task>& vec_);
        std::list<task> swap_to_list();
        std::vector<task> swap_to_vector();

        /** move copy **/
        lfthread& operator=(lfthread&& other) noexcept;
        lfthread(lfthread&& other) noexcept;

    private:
        void t_task();

    private:
        ringbuff<task> buffer_;  // 任务队列(无锁环形缓冲区)
        bool shutdown_;  // 无任何其他线程操作，所以不需要原子变量
        std::thread t_;
    };

}  // namespace moon

#endif
