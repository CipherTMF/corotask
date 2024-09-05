#pragma once
#include <chrono>

namespace ct::common {
   template <typename F>
   auto time_it(F&& f) requires (!std::is_same_v<void, std::invoke_result_t<F>>){
      auto begin = std::chrono::steady_clock::now();
      auto result = f();
      auto end = std::chrono::steady_clock::now();
      auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
      return std::make_pair(result, ns);
   }

   template <typename F>
   auto time_it(F&& f) requires (std::is_same_v<void, std::invoke_result_t<F>>){
      auto begin = std::chrono::steady_clock::now();
      f();
      auto end = std::chrono::steady_clock::now();
      auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
      return ns;
   }
}