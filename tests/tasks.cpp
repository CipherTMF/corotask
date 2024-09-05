#include <doctest/doctest.h>
#include <corotask/corotask>

using namespace ct::tasks;

TEST_SUITE("tasks") {
   TEST_CASE("task creation") {
      auto fib = [](auto&& self, const int x) -> task<int> {
         if (x <= 2) co_return 1;
         co_return co_await self(self, x - 1) + co_await self(self, x - 2);
      };

      int result = fib(fib, 10).get();
      CHECK(result == 55);
   }
}