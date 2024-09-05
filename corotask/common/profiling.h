#pragma once
#include <array>
#include <functional>
#include <unordered_set>
#include <thread>

namespace ct::common {

   constexpr size_t MAX_PROFILING_STACK_DEPTH = 2048;

   struct profiling_data {
      const char* category = nullptr;
      const char* location = nullptr;
      const char* name = nullptr;
      uint64_t call_count = 0;
      uint64_t total_nanos = 0;
      uint64_t children_nanos = 0;
   };

   class profiling_registry {
   public:
      static profiling_registry& instance();

      void register_instance(profiling_data(*getter)());
      void start_recording(profiling_data* data);
      void stop_recording();
      void print_data();

   private:
      std::vector<profiling_data> collect();
      void print(std::vector<profiling_data>&& accumulated);

      struct stack_item {
         std::chrono::high_resolution_clock::time_point start;
         profiling_data* data;
      };

      profiling_registry() = default;
      std::unordered_set<profiling_data(*)()> _registry;
      std::mutex _tex;
      std::unordered_map<std::thread::id, std::array<stack_item, MAX_PROFILING_STACK_DEPTH>> _stack;
      std::unordered_map<std::thread::id, size_t> _stack_size;
   };

   template<std::size_t n>
   struct fixed_string {
      constexpr fixed_string() = default;

      constexpr fixed_string(const char (& str)[n + 1]) noexcept {
         auto i = std::size_t{0};
         for (char const c: str) {
            _data[i++] = c;
         }
         _data[n] = static_cast<char>(0);
      }

      friend constexpr auto operator<=>(fixed_string const&, fixed_string const&) = default;
      friend constexpr auto operator==(fixed_string const&, fixed_string const&) -> bool = default;

      [[nodiscard]]
      static constexpr auto size() noexcept -> std::size_t {
         return n;
      }

      [[nodiscard]]
      static constexpr auto empty() noexcept -> bool {
         return n == 0;
      }

      constexpr auto data() const & noexcept -> char const* {
         return _data;
      }

      constexpr auto data() & noexcept -> char* {
         return _data;
      }

      constexpr auto begin() const & noexcept -> char const* {
         return _data;
      }

      constexpr auto end() const & noexcept -> char const* {
         return _data + n;
      }

      constexpr auto begin() & noexcept -> char* {
         return _data;
      }

      constexpr auto end() & noexcept -> char* {
         return _data + n;
      }

      constexpr auto operator[](std::size_t index) noexcept {
         return _data[index];
      }

      char _data[n + 1];
   };

   // template argument deduction
   template<std::size_t n>
   fixed_string(char const (& )[n]) -> fixed_string<n - 1>;

   constexpr size_t str_size(const char* str) {
      size_t s = 0;
      while (str[s++] != '\0') {}
      return s;
   }

   constexpr const char* fileName(const char* filePath) {
      const size_t size = str_size(filePath);
      for (int i = size - 1; i >= 0; i--) {
         if (filePath[i] == '/' || filePath[i] == '\\') {
            return filePath + i + 1;
         }
      }
      return nullptr;
   }

   template<fixed_string Category, fixed_string Location, fixed_string FuncName = "">
   class raii_profiler {
      static inline profiling_data data = [] {
         profiling_data data;
         data.category = Category.data();
         data.location = fileName(Location.data());
         data.name = FuncName.data();
         return data;
      }();

      static inline bool is_registered = [] {
         profiling_registry::instance().register_instance(+[] {
            return data;
         });
         return true;
      }();

   public:
      raii_profiler() {
         if (!is_registered) { std::abort(); }
         profiling_registry::instance().start_recording(&data);
      }

      raii_profiler(const raii_profiler&) = delete;
      raii_profiler(raii_profiler&&) = delete;
      raii_profiler& operator=(const raii_profiler&) = delete;
      raii_profiler& operator=(raii_profiler&&) = delete;

      ~raii_profiler() {
         profiling_registry::instance().stop_recording();
      }
   };
}

#if defined(_MSC_VER)
#define CT_FUNCTION_NAME __FUNCTION__
#else
#define CT_FUNCTION_NAME __func__
#endif

#define _CT_CAT(a, b) a##b
#define CT_CAT(a, b) _CT_CAT(a, b)
#define _CT_STRINGIFY(a) #a
#define CT_STRINGIFY(a) _CT_STRINGIFY(a)

#define CT_FUNC_LOC __FILE__ ":" CT_STRINGIFY(__LINE__)
#define CT_FUNC_PROFILER(CATEGORY) ct::common::raii_profiler<CATEGORY, CT_FUNC_LOC, CT_FUNCTION_NAME> CT_CAT(_profiler_, __LINE__)
#define CT_MIXED_PROFILER(CATEGORY, FUNCNAME) ct::common::raii_profiler<CATEGORY, CT_FUNC_LOC, FUNCNAME> CT_CAT(_profiler_, __LINE__)
