#pragma once

#include <QObject>
#include <vector>
#include <functional>

#include <corotask/ctl/list.hpp>

namespace ct::threads {
   class coro_pool : public QObject {
      Q_OBJECT

   public:
      coro_pool(size_t amount_workers = 0);

      void shutdown();

      void add_workers(size_t amount);
      void remove_workers(size_t amount);
      size_t workers_count() const;

      void add_coroutine(void* coro);
      void* steal_coroutine(void* coro);

   private:
      void worker_loop(void* coro = nullptr);

   private:
      ctl::list<QThread*> _workers;
      ctl::list<void*> _coros;
   };
}
