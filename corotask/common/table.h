#pragma once

#include <vector>
#include <string>

namespace ct::common {
   class table {
   public:
      // Add a row to the table
      void addRow(const std::vector<std::string>& row);
      void addRow(std::vector<std::string>&& row);
      void addLine();

      // Print the table
      void print() const;

   private:
      // Print a single row with proper spacing
      void printRow(const std::vector<std::string>& row) const;

   private:
      std::vector<std::vector<std::string>> rows; // Store table rows (each row is a vector of strings)
      std::vector<size_t> columnWidths; // Store column widths
   };
}
