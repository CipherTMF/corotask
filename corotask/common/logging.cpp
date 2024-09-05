#include "logging.h"

#include <iostream>
#include <QDateTime>

using namespace ct::logging;

logger& logger::instance() {
   static logger inst;
   return inst;
}

void logger::log(const level lvl, const QString& msg) {
   if (_level > lvl) return;

   const auto current_time = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);

   switch (lvl) {
      case VERBOSE:
         print(colors::WHITE, colors::STAY, colors::STAY, current_time + ":[VERBOSE]: " + msg);
         break;
      case INFO:
         print(colors::GREEN, colors::STAY, colors::STAY, current_time + ":[INFO]: " + msg);
         break;
      case WARNING:
         print(colors::YELLOW, colors::STAY, colors::STAY, current_time + ":[WARNING]: " + msg);
         break;
      case ERROR:
         print(colors::RED, colors::STAY, colors::STAY, current_time + ":[ERROR]: " + msg);
         break;
      case FATAL:
         print(colors::RED, colors::BG_YELLOW, colors::UNDERLINE, current_time + ":[FATAL]: " + msg);
         break;
   }
}

void logger::print(const char* text, const char* bg, const char* style, const QString& msg) {
   std::cout << text << bg << style << msg.toStdString() << colors::RESET << std::endl;
}

void logger::set_level(const level lvl) {
   _level = lvl;
}
