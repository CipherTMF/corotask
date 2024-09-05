#pragma once

#include "corotask/tasks/task.hpp"
#include "corotask/model/rank.h"

namespace ct::interface {
   template<typename T>
   T exec(tasks::task<T> f) {
      // enqueue task
      void* address = f.handle.address();
      model::rank::instance().spawn_root_task(std::move(f));

      // wait for task finished
      model::rank::instance().await(address);

      // return result
      auto handle = std::coroutine_handle<tasks::promise<T> >::from_address(address);
      if constexpr (!std::is_same_v<T, void>) {
         T result = std::move(*handle.promise().value);
         handle.destroy();
         return result;
      } else {
         handle.destroy();
         return;
      }
   }
}
