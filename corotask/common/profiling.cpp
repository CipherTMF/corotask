#include "profiling.h"

#include <iomanip>
#include <iostream>
#include <stack>
#include "table.h"
using namespace ct::common;

namespace {
   class custom_thousands_separator : public std::numpunct<char> {
   protected:
      char do_thousands_sep() const override { return ','; } // Use dot as separator
      std::string do_grouping() const override { return "\3"; } // Group every 3 digits
   };

   std::string double_to_string(double value, int precision) {
      std::stringstream ss;
      ss.imbue(std::locale(std::locale::classic(), new custom_thousands_separator));
      ss << std::fixed << std::setprecision(precision) << value;
      return ss.str();
   }

   template<typename T>
   std::string to_string(T v) {
      std::stringstream ss;
      ss.imbue(std::locale(std::locale::classic(), new custom_thousands_separator));
      ss << v;
      return ss.str();
   }
}

profiling_registry& profiling_registry::instance() {
   static profiling_registry inst;
   return inst;
}

void profiling_registry::register_instance(profiling_data (* getter)()) {
   _registry.emplace(getter);
}

void profiling_registry::start_recording(profiling_data* data) {
   std::array<stack_item, MAX_PROFILING_STACK_DEPTH>* stack = nullptr;
   size_t* stack_size = nullptr;

   if (!_stack.contains(std::this_thread::get_id())) {
      std::scoped_lock lock(_tex);
      _stack[std::this_thread::get_id()] = {};
      _stack_size[std::this_thread::get_id()] = 0;
      stack = &_stack[std::this_thread::get_id()];
      stack_size = &_stack_size[std::this_thread::get_id()];
   } else {
      std::scoped_lock lock(_tex);
      stack = &_stack[std::this_thread::get_id()];
      stack_size = &_stack_size[std::this_thread::get_id()];
   }

   (*stack)[(*stack_size)++] = {
      .start = std::chrono::high_resolution_clock::now(),
      .data = data
   };
   data->call_count++;
}

void profiling_registry::stop_recording() {
   std::array<stack_item, MAX_PROFILING_STACK_DEPTH>* stack = nullptr;
   size_t* stack_size = nullptr; {
      std::scoped_lock lock(_tex);
      stack = &_stack[std::this_thread::get_id()];
      stack_size = &_stack_size[std::this_thread::get_id()];
   }
   const stack_item item = (*stack)[--(*stack_size)];
   const uint64_t ns_ran = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::high_resolution_clock::now() - item.start).count();
   item.data->total_nanos += ns_ran;
   if (*stack_size > 0) {
      (*stack)[(*stack_size) - 1].data->children_nanos += ns_ran;
   }
}

void profiling_registry::print_data() {
   print(collect());
}

std::vector<profiling_data> profiling_registry::collect() {
   std::vector<profiling_data> result;
   for (auto& getter: _registry) {
      result.push_back(getter());
   }
   return result;
}

void profiling_registry::print(std::vector<profiling_data>&& accumulated) {
   std::unordered_map<std::string, std::vector<profiling_data> > by_cat;
   for (auto&& data: std::move(accumulated)) {
      by_cat[data.category].emplace_back(std::move(data));
   }

   // inner sort by total runtime
   for (auto& [cat, list]: by_cat) {
      std::sort(list.begin(), list.end(), [](const profiling_data& lhs, const profiling_data& rhs) {
         return lhs.total_nanos > rhs.total_nanos;
      });
   }

   // outer sort by category
   std::vector<std::pair<std::string, std::vector<profiling_data> > > data(by_cat.begin(), by_cat.end());
   std::sort(data.begin(), data.end(), [](const std::pair<std::string, std::vector<profiling_data> >& lhs,
                                          const std::pair<std::string, std::vector<profiling_data> >& rhs) {
      return lhs.first > rhs.first;
   });

   table tbl;
   tbl.addRow(std::vector<std::string>{
      "category", "name", "location", "total time", "exclusive time", "# calls", "avg total", "avg exclusive",
      "% total", "% exclusive"
   });

   profiling_data* root = nullptr;
   for (auto& dp : by_cat["corotask/interface"]) {
      if (dp.name == std::string("root")) {
         root = &dp;
         break;
      }
   }
   const uint64_t max_total_nanos = std::thread::hardware_concurrency() * root->total_nanos;

   for (auto& [cat, dl]: data) {
      tbl.addLine();
      for (auto& dp: dl) {
         std::vector<std::string> row;
         row.push_back(cat);
         row.push_back(dp.name);
         row.push_back(dp.location);
         row.push_back(to_string(dp.total_nanos));
         row.push_back(to_string(dp.total_nanos - dp.children_nanos));
         row.push_back(to_string(dp.call_count));
         row.push_back(double_to_string(dp.total_nanos / (double) dp.call_count, 2));
         row.push_back(double_to_string((dp.total_nanos - dp.children_nanos) / (double) dp.call_count, 2));
         row.push_back(double_to_string((double) dp.total_nanos * 100 / max_total_nanos, 2));
         row.push_back(double_to_string((double) (dp.total_nanos - dp.children_nanos) * 100 / max_total_nanos, 2));
         tbl.addRow(std::move(row));
      }
   }

   tbl.print();
}
