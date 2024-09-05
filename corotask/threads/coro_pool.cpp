#include "coro_pool.h"
#include <coroutine>
#include <QThread>
#include <corotask/tasks/task.hpp>
#include <corotask/common/profiling.h>

using namespace ct::threads;

coro_pool::coro_pool(const size_t amount_workers) {
   CT_FUNC_PROFILER("corotask/scheduler");
   add_workers(amount_workers);
}

void coro_pool::shutdown() {
   CT_FUNC_PROFILER("corotask/scheduler");
   remove_workers(_workers.size());
}

void coro_pool::add_workers(const size_t amount) {
   CT_FUNC_PROFILER("corotask/scheduler");
   for (size_t i = 0; i < amount; i++) {
      auto* thread = new QThread(this);
      thread->setObjectName("QThread #" + std::to_string(_workers.size()));
      connect(thread, &QThread::started, [this] {
         while (!QThread::currentThread()->isInterruptionRequested()) {
            this->worker_loop(nullptr);
         }
      });
      thread->start();
      _workers.add_back(thread);
   }
}

void coro_pool::remove_workers(const size_t amount) {
   CT_FUNC_PROFILER("corotask/scheduler");
   for (size_t i = 0; i < amount; i++) {
      QThread* worker = _workers.take_back();
      worker->requestInterruption();
      worker->quit();
      worker->wait();
      delete worker;
   }
}

size_t coro_pool::workers_count() const {
   CT_FUNC_PROFILER("corotask/scheduler");
   return _workers.size();
}

void coro_pool::add_coroutine(void* coro) {
   CT_FUNC_PROFILER("corotask/scheduler");
   _coros.add_front(coro);
}

void coro_pool::worker_loop(void* coro) {
   CT_FUNC_PROFILER("corotask/scheduler");
   // CT_VERBOSE(QString("size = %1").arg(_coros.size()));
   // continue with given coroutine or take one
   auto* _coro = coro ? coro : _coros.take_front();

   // check if work available TODO: workstealing here
   if (_coro == nullptr) {
      return;
   }

   // check resumable
   auto& prom_base = tasks::promise_base_generic::from_address(_coro);
   if (prom_base.awaiting_task_count > 0) {
      // not resumable yet, requeue
      _coros.add_back(_coro);
      return;
   }

   const auto handle = std::coroutine_handle<>::from_address(_coro);

   // check if done
   if (handle.done()) {
      // it's done, nothing to do here
      return;
   }

   // check if coroutine can be worked on
   if (!std::atomic_flag_test_and_set(&prom_base.is_worked_on)) {
      CT_MIXED_PROFILER("corotask/exec", "resume");
      handle.resume();
   }

   // check if done
   if (handle.done()) {
      // try to work on parent
      prom_base.is_done.test_and_set();
      auto parent = prom_base.parent.load();
      if (parent != nullptr) {
         auto& pprom_base = tasks::promise_base_generic::from_address(parent);
         --pprom_base.awaiting_task_count;
      }
      std::atomic_flag_clear(&prom_base.is_worked_on);
      // worker_loop(prom_base.parent);
      return;
   }

   // requeue
   _coros.add_back(_coro);
   std::atomic_flag_clear(&prom_base.is_worked_on);
}
