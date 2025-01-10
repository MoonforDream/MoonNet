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

#ifndef _LFTHREADPOOL_H_
#define _LFTHREADPOOL_H_

#include <functional>
#include <vector>
#include <sys/types.h>
#include <thread>
#include <atomic>

#define ADJUST_TIMEOUT_SEC 5

namespace moon {

    class lfthread;

    enum class PoolMode {
        Static,  // 静态模式
        Dynamic  // 动态模式
    };

    class lfthreadpool {
    public:
        lfthreadpool(int tnum = -1, size_t buffsize = 1024,
                     PoolMode mode = PoolMode::Static);
        ~lfthreadpool();
        void init();
        void t_shutdown();

        /** add_task function **/
        template <typename _Fn, typename... _Args>
        bool add_task(_Fn&& fn, _Args&&... args);

        template <typename _Fn, typename... _Args>
        bool add_task_move(_Fn&& fn, _Args&&... args);

        lfthreadpool(const lfthreadpool&) = delete;
        lfthreadpool& operator=(const lfthreadpool&) = delete;

    private:
        const size_t getnext();
        void adjust_task();
        void del_thread_dispath();
        void add_thread();
        const size_t getminidx();
        const size_t getmaxidx();
        const int getload();
        void setnum();
        void setnum(size_t n);
        int getcores() const;

    private:
        std::atomic<size_t> tnum_;
        // size_t tnum_;
        std::thread mentor_;
        std::vector<lfthread*> workers_;
        // bool shutdown_;
        std::atomic<bool> shutdown_;
        PoolMode mode_;
        size_t buffsize_;
        size_t next_ = 0;
        int timesec_ = 5;
        int coolsec_ = 30;
        int load_max = 80;
        int load_min = 20;
        int max_tnum = 0;
        int min_tnum = 0;
    };

}  // namespace moon

#include "lfthreadpool.tpp"

#endif  // !_LFTHREADPOOL_H_
