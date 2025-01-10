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

// lfthreadpool.tpp
#ifndef LFTHREADPOOL_TPP
#define LFTHREADPOOL_TPP

#include "lfthread.h"

namespace moon {

    template <typename _Fn, typename... _Args>
    bool lfthreadpool::add_task(_Fn&& fn, _Args&&... args) {
        if (shutdown_.load(std::memory_order_acquire)) return false;
        auto f = std::bind(std::forward<_Fn>(fn), std::forward<_Args>(args)...);
        size_t idx = getnext();
        if (idx >= workers_.size()) return false;  // 防止越界
        //        return workers_[idx]->enqueue_task(std::move(f));
        return workers_[idx]->enqueue_task(f);
    }

    template <typename _Fn, typename... _Args>
    bool lfthreadpool::add_task_move(_Fn&& fn, _Args&&... args) {
        if (shutdown_.load(std::memory_order_acquire)) return false;
        auto f = std::bind(std::forward<_Fn>(fn), std::forward<_Args>(args)...);
        size_t idx = getnext();
        if (idx >= workers_.size()) return false;  // 防止越界
        return workers_[idx]->enqueue_task_move(std::move(f));
    }
}  // namespace moon

#endif  // LFTHREADPOOL_TPP
