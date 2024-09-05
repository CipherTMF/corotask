#include <iostream>
#include <QCoreApplication>
#include <QTimer>
#include <corotask/corotask>

int main(int argc, char** argv) {
   auto app = QCoreApplication(argc, argv);

   QTimer::singleShot(0, [] {
      ct::logging::logger::instance().set_level(ct::logging::level::VERBOSE);
      CT_VERBOSE("NOT IMPLEMENTED YET");
      CT_INFO("NOT IMPLEMENTED YET");
      CT_WARNING("NOT IMPLEMENTED YET");
      CT_ERROR("NOT IMPLEMENTED YET");
      CT_FATAL("NOT IMPLEMENTED YET");
      qApp->exit();
   });

   return app.exec();
}