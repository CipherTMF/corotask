#pragma once

#include <cstdlib>
#include <QCoreApplication>
#include <QTimer>
#include <corotask/model/rank.h>

namespace ct::interface {
   template<typename F>
   int root(int argc, char** argv, F&& f) {
      QCoreApplication app(argc, argv);

      model::rank::instance().init(argc, argv);

      QTimer::singleShot(0, [f] {
         {
            CT_MIXED_PROFILER("corotask/interface", "root");
            f();

            model::rank::instance().fini();
         }

         ct::common::profiling_registry::instance().print_data();

         qApp->exit(0);
      });
      return app.exec();
   }
}
