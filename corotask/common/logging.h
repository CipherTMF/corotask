#pragma once

#include <QString>
#include <QObject>

namespace ct::logging {
   enum level : unsigned { VERBOSE, INFO, WARNING, ERROR, FATAL };
   struct colors {
      static constexpr auto RESET = "\033[0m";
      static constexpr auto STAY = "";

      // Text Colors
      static constexpr auto BLACK = "\033[30m";
      static constexpr auto RED = "\033[31m";
      static constexpr auto GREEN = "\033[32m";
      static constexpr auto YELLOW = "\033[33m";
      static constexpr auto BLUE = "\033[34m";
      static constexpr auto MAGENTA = "\033[35m";
      static constexpr auto CYAN = "\033[36m";
      static constexpr auto WHITE = "\033[37m";

      // Background Colors
      static constexpr auto BG_BLACK = "\033[40m";
      static constexpr auto BG_RED = "\033[41m";
      static constexpr auto BG_GREEN = "\033[42m";
      static constexpr auto BG_YELLOW = "\033[43m";
      static constexpr auto BG_BLUE = "\033[44m";
      static constexpr auto BG_MAGENTA = "\033[45m";
      static constexpr auto BG_CYAN = "\033[46m";
      static constexpr auto BG_WHITE = "\033[47m";

      // Text Attributes
      static constexpr auto BOLD = "\033[1m";
      static constexpr auto UNDERLINE = "\033[4m";
      static constexpr auto REVERSED = "\033[7m";
   };

   class logger : public QObject {
      Q_OBJECT

   public:
      static logger& instance();

      static void print(const char* text, const char* bg, const char* style, const QString& msg);

      void log(level lvl, const QString& msg);
      void set_level(level lvl);

   private:
      using QObject::QObject;

      level _level = VERBOSE;
   };
}

#define CT_VERBOSE(MSG) ct::logging::logger::instance().log(ct::logging::VERBOSE, MSG)
#define CT_INFO(MSG) ct::logging::logger::instance().log(ct::logging::INFO, MSG)
#define CT_WARNING(MSG) ct::logging::logger::instance().log(ct::logging::WARNING, MSG)
#define CT_ERROR(MSG) ct::logging::logger::instance().log(ct::logging::ERROR, MSG)
#define CT_FATAL(MSG) ct::logging::logger::instance().log(ct::logging::FATAL, MSG)
