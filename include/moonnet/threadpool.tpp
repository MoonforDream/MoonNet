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

#ifndef _THREADPOOL_TPP_
#define _THREADPOOL_TPP_

namespace moon {

    /**
     * @brief Adds a new task to the thread pool.
     *
     * This templated function allows users to add tasks with varying arguments
     * to the thread pool. It binds the provided function and arguments into a
     * callable task, ensures the thread pool is not shutting down, and notifies
     * one of the worker threads to execute the task.
     *
     * @tparam _Fn The type of the function to be added as a task.
     * @tparam _Args The types of the arguments to be passed to the function.
     * @param fn The function to be executed by a worker thread.
     * @param args The arguments to be passed to the function.
     */
    template <typename _Fn, typename... _Args>
    void threadpool::add_task(_Fn&& fn, _Args&&... args) {
        {
            auto f =
                std::bind(std::forward<_Fn>(fn), std::forward<_Args>(args)...);
            {
                std::unique_lock<std::mutex> lock(mx);
                if (shutdown) return;
                tasks.emplace(std::move(f));
            }
            task_cv.notify_one();
        }
    }
}  // namespace moon

#endif  // !_THREADPOOL_TPP_
