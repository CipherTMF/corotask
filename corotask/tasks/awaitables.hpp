#pragma once

#include "task.hpp"
#include <corotask/model/rank.h>

namespace ct::tasks {
   template<typename T> struct promise;

   template<typename T>
   struct task_awaiter {
      std::coroutine_handle<promise<T> > handle;

      constexpr static bool await_ready() noexcept { return false; }

      template<typename U>
      void await_suspend(std::coroutine_handle<promise<U> > parent) const noexcept {
         CT_FUNC_PROFILER("corotask/fork");
         model::rank::instance().spawn_tasks(parent, handle);
      }

      T await_resume() const noexcept {
         CT_FUNC_PROFILER("corotask/join");
         T result = std::move(*handle.promise().value);
         handle.destroy();
         return result;
      }
   };

   template<typename T>
   template<typename U>
   task_awaiter<U> promise_base<T>::await_transform(task<U> task) {
      CT_FUNC_PROFILER("corotask/fork");
      return task_awaiter<U>{.handle = task.handle};
   }
}
