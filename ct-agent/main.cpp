#include <iostream>
#include <QCoreApplication>
#include <QTimer>
#include <corotask/corotask>

int main(int argc, char** argv) {
   auto app = QCoreApplication(argc, argv);

   auto server = ct::network::server();

   QTimer::singleShot(0, []() {
      qInfo() << "Ending";
      qApp->exit();
   });

   return app.exec(); // NOLINT(*-static-accessed-through-instance)
}