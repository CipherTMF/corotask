#include "table.h"

#include <iomanip>
#include <iostream>
#include <numeric>
using namespace ct::common;

void table::addRow(const std::vector<std::string>& row) {
   rows.push_back(row);

   // Update column widths based on the new row
   if (columnWidths.size() < row.size()) {
      columnWidths.resize(row.size(), 0); // Ensure there are enough columns
   }
   for (size_t i = 0; i < row.size(); ++i) {
      columnWidths[i] = std::max(columnWidths[i], row[i].size());
   }
}

void table::addRow(std::vector<std::string>&& row) {
   rows.push_back(std::move(row));

   // Update column widths based on the new row
   if (columnWidths.size() < rows.back().size()) {
      columnWidths.resize(rows.back().size(), 0); // Ensure there are enough columns
   }
   for (size_t i = 0; i < rows.back().size(); ++i) {
      columnWidths[i] = std::max(columnWidths[i], rows.back()[i].size());
   }
}

void table::addLine() {
   rows.push_back({});
}

void table::print() const {
   if (rows.empty()) return;

   // Print each row with proper formatting
   for (const auto& row: rows) {
      printRow(row);
   }
}

void table::printRow(const std::vector<std::string>& row) const {
   if (row.empty()) {
      // print horizontal line
      const int sum = std::accumulate(columnWidths.begin(), columnWidths.end(), 0,[](int sum, int col) {
         return sum + col + 2;
      });
      std::cout << std::string(sum, '-') << std::endl;
   } else {
      for (size_t i = 0; i < row.size(); ++i) {
         std::cout << std::setw(columnWidths[i] + 2) << (i <= 2 ? std::left : std::right) << row[i]; // Left align with padding
      }
      std::cout << std::endl;
   }
}
