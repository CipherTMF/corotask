#include <iostream>
#include <corotask/corotask>
#include <QThread>

#define ITERS 10

template<typename T, size_t S>
ct::task<T> add(std::array<T, S> nums) {
   if constexpr (S == 0) {
      co_return 0;
   } else if constexpr (S == 1) {
      co_return nums[0];
   } else {
      constexpr auto s1 = S / 2;
      constexpr auto s2 = S - s1;
      std::array<T, s1> left;
      std::copy(nums.cbegin(), nums.cbegin() + s1, left.begin());
      std::array<T, s2> right;
      std::copy(nums.cbegin() + s1, nums.cend(), right.begin());
      co_return co_await add(left) + co_await add(right);
   }
}

template<typename T, size_t S>
T add_seq(std::array<T, S> nums) {
   if constexpr (S == 0) {
      return 0;
   } else if constexpr (S == 1) {
      return nums[0];
   } else {
      constexpr auto s1 = S / 2;
      constexpr auto s2 = S - s1;
      std::array<T, s1> left;
      std::copy(nums.cbegin(), nums.cbegin() + s1, left.begin());
      std::array<T, s2> right;
      std::copy(nums.cbegin() + s1, nums.cend(), right.begin());
      return add_seq(left) + add_seq(right);
   }
}

volatile int result = 0;
const auto INPUT = [](){
   std::array<int, 1'000> nums;
   for (int i = 0; i < nums.size(); i++)
      nums[i] = i;
   return nums;
}();

int main(int argc, char** argv) {
   return ct::root(argc, argv,
                   [] {
                      auto ns_seq = ct::time_it([] {
                         ct::exec([]() -> ct::task<void> {
                            for (int i = 0; i < ITERS; i++) {
                               result = add_seq(INPUT);
                            }
                            co_return;
                         }());
                      });
                      auto ns = ct::time_it([] {
                         ct::exec([]() -> ct::task<void> {
                            for (int i = 0; i < ITERS; i++) {
                               result = co_await add(INPUT);
                            }
                            co_return;
                         }());
                      });
                      std::cout << "tasks (time = " << ns << " ns)" << std::endl; // 55
                      std::cout << "sequential (time = " << ns_seq << " ns)" << std::endl; // 55
                   });
}
