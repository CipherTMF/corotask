#pragma once

#include <memory>

namespace ct {
   template <typename T> using sptr = std::shared_ptr<T>;
   template <typename T> using uptr = std::unique_ptr<T>;
   template <typename T> using wptr = std::weak_ptr<T>;
}