#pragma once

#include <QThread>
#include <corotask/common/profiling.h>

namespace ct::ctl {

   template<typename T, typename Allocator = std::allocator<T> >
   class list {
   public:
      list();
      list(const list& other);
      list(list&& other) noexcept;
      list& operator=(const list& other);
      list& operator=(list&& other) noexcept;
      ~list();

      size_t size() const;

      void add_front(T data);
      void add_back(T data);

      T take_front();
      T take_back();

   private:
      mutable std::mutex _tex;
      std::list<T, Allocator> _list;
   };

   template<typename T, typename Allocator> list<T, Allocator>::list() {
   }

   template<typename T, typename Allocator> list<T, Allocator>::list(const list& other) {
      std::scoped_lock lock(_tex, other._tex);
      for (auto& item: other._list) {
         add_back(item);
      }
   }

   template<typename T, typename Allocator> list<T, Allocator>::list(list&& other) noexcept {
      std::scoped_lock lock(_tex, other._tex);
      _list = std::move(other._list);
   }

   template<typename T, typename Allocator> list<T, Allocator>& list<T, Allocator>::operator=(const list& other) {
      std::scoped_lock lock(_tex, other._tex);
      for (auto& item: other._list) {
         add_back(item);
      }
      return *this;
   }

   template<typename T, typename Allocator> list<T, Allocator>& list<T, Allocator>::operator=(list&& other) noexcept {
      std::scoped_lock lock(_tex, other._tex);
      if (this != &other) {
         _list = std::move(other._list);
      }
      return *this;
   }

   template<typename T, typename Allocator> list<T, Allocator>::~list() {
      CT_FUNC_PROFILER("corotask/list");
   }

   template<typename T, typename Allocator> size_t list<T, Allocator>::size() const {
      return _list.size();
   }

   template<typename T, typename Allocator> void list<T, Allocator>::add_front(T data) {
      CT_FUNC_PROFILER("corotask/list");
      std::scoped_lock lock(_tex);
      _list.push_front(data);
   }

   template<typename T, typename Allocator> void list<T, Allocator>::add_back(T data) {
      CT_FUNC_PROFILER("corotask/list");
      std::scoped_lock lock(_tex);
      _list.push_back(data);
   }

   template<typename T, typename Allocator> T list<T, Allocator>::take_front() {
      CT_FUNC_PROFILER("corotask/list");
      std::scoped_lock lock(_tex);
      if (_list.empty()) return nullptr;
      auto front = _list.front();
      _list.pop_front();
      return front;
   }

   template<typename T, typename Allocator> T list<T, Allocator>::take_back() {
      CT_FUNC_PROFILER("corotask/list");
      std::scoped_lock lock(_tex);
      if (_list.empty()) return nullptr;
      auto back = _list.back();
      _list.pop_back();
      return back;
   }
}
