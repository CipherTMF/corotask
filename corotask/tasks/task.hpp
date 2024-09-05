#pragma once

#include <coroutine>
#include <optional>
#include <cstdint>

namespace ct::tasks {
   static uint8_t PROMISE_OFFSET = []() -> uint8_t {
      struct base_task {
         struct promise_type {
            static constexpr std::suspend_always initial_suspend() noexcept { return {}; }
            static constexpr std::suspend_always final_suspend() noexcept { return {}; }

            static constexpr void unhandled_exception() noexcept {
            }

            static constexpr void return_void() noexcept {
            }

            base_task get_return_object() noexcept {
               return {.handle = std::coroutine_handle<promise_type>::from_promise(*this)};
            }
         };

         std::coroutine_handle<promise_type> handle;
      };
      const auto coro = []() -> base_task { co_return; };
      const auto [handle] = coro();
      const auto* base = static_cast<std::byte*>(handle.address());
      const auto* prom = reinterpret_cast<std::byte*>(&handle.promise());
      return prom - base;
   }();

   template<typename T> struct task;
   template<typename T> struct promise_base;
   template<typename T> struct promise;
   template<typename T> struct task_awaiter;

   template<typename T>
   struct task {
      using value_type = T;
      using promise_type = promise<T>;

      std::coroutine_handle<promise_type> handle;

      T get() const {
         while (!handle.done()) handle.resume();
         if constexpr (std::is_same_v<T, void>) {
            return;
         } else {
            return std::move(*handle.promise().value);
         }
      }
   };

#pragma pack(push, 1)
   struct promise_base_generic {
      std::atomic<void*> parent = nullptr; // 8 byte
      std::atomic<uint16_t> awaiting_task_count = 0; // 2 byte
      std::atomic_flag is_worked_on = false; // 1 byte
      std::atomic_flag is_done = false; // 1 byte
      uint32_t unused: 32 = 0;

      static promise_base_generic& from_address(void* const ptr) {
         const auto bptr = static_cast<std::byte*>(ptr);
         const auto prom_ptr = bptr + PROMISE_OFFSET;
         return *reinterpret_cast<promise_base_generic*>(prom_ptr);
      }

      static promise_base_generic& from_handle(const std::coroutine_handle<> handle) {
         return from_address(handle.address());
      }
   };
#pragma pack(pop)

   static_assert(sizeof(promise_base_generic) == 16, "promise_base_generic must be exactly 16 bytes in size");

   template<typename T>
   struct promise_base : promise_base_generic {
      static constexpr std::suspend_always initial_suspend() noexcept { return {}; }
      static constexpr std::suspend_always final_suspend() noexcept { return {}; }
      static constexpr void unhandled_exception() noexcept { return; }
      [[nodiscard]] task<T> get_return_object() noexcept;

      template<typename U>
      task_awaiter<U> await_transform(task<U> task);
   };

   template<typename T>
   struct promise : promise_base<T> {
      using promise_base<T>::promise_base;
      std::optional<T> value;
      void return_value(T value) noexcept { this->value = std::move(value); }
   };

   template<>
   struct promise<void> : promise_base<void> {
      using promise_base<void>::promise_base;

      static constexpr void return_void() noexcept {
      }
   };

   template<typename T>
   task<T> promise_base<T>::get_return_object() noexcept {
      return {std::coroutine_handle<promise<T> >::from_promise(*static_cast<promise<T>*>(this))};
   }
}
