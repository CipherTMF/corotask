#pragma once

#include <cstdint>
#include <vector>

#include "corotask/common/traits.h"
#include "corotask/network/server.h"
#include "corotask/network/client.h"
#include "corotask/threads/coro_pool.h"
#include "corotask/tasks/task.hpp"

namespace ct::model {
   static constexpr auto PORT = 35355;

   class rank : public QObject {
      Q_OBJECT

   public:
      static rank& instance();

      bool init(int argc, char** argv);
      bool fini();

      template<typename P, typename... Ts>
      void spawn_tasks(tasks::task<P> parent, tasks::task<Ts>&&... children);
      template<typename P, typename... Ts>
      void spawn_tasks(std::coroutine_handle<tasks::promise<P> > parent,
                       std::coroutine_handle<tasks::promise<Ts> >&&... children);
      template<typename P, typename T>
      void spawn_tasks(std::coroutine_handle<tasks::promise<P> > parent,
                       std::coroutine_handle<tasks::promise<T> > child);
      template<typename T>
      void spawn_root_task(tasks::task<T> task);

      void await(void* coro);

   private:
      bool init_root(int argc, char** argv);
      bool init_worker(int argc, char** argv);

      using QObject::QObject;

      uint8_t _id = 0;
      uint8_t _threads = 0;
      uint8_t _workers = 0;
      uptr<network::server> _server = nullptr;
      std::vector<uptr<network::client> > _clients = {};
      threads::coro_pool _coro_pool;
   };

   template<typename P, typename... Ts>
   void rank::spawn_tasks(tasks::task<P> parent, tasks::task<Ts>&&... children) {
      parent.handle.promise().awaiting_task_count += sizeof...(Ts);
      auto set_parent = [this, parent, children = std::make_tuple(children...)]<size_t ... Is
            >(std::index_sequence<Is...>) {
         ((std::get<Is>(children).handle.promise().parent = parent.handle.address()), ...);
         ((_coro_pool.add_coroutine(std::get<Is>(children).handle.address())), ...);
      };
      set_parent(std::index_sequence_for<Ts...>());
   }

   template<typename P, typename... Ts>
   void rank::spawn_tasks(std::coroutine_handle<tasks::promise<P> > parent,
                          std::coroutine_handle<tasks::promise<Ts> >&&... children) {
      parent.promise().awaiting_task_count += sizeof...(Ts);
      auto set_parent = [this, parent, children = std::make_tuple(children...)]<size_t ... Is
            >(std::index_sequence<Is...>) {
         ((std::get<Is>(children).promise().parent = parent.address()), ...);
         ((_coro_pool.add_coroutine(std::get<Is>(children).address())), ...);
      };
      set_parent(std::index_sequence_for<Ts...>());
   }

   template<typename P, typename T> void rank::spawn_tasks(std::coroutine_handle<tasks::promise<P> > parent,
                                                           std::coroutine_handle<tasks::promise<T> > child) {
      ++parent.promise().awaiting_task_count;
      child.promise().parent = parent.address();
      _coro_pool.add_coroutine(child.address());
   }

   template<typename T>
   void rank::spawn_root_task(tasks::task<T> task) {
      _coro_pool.add_coroutine(task.handle.address());
   }
}
