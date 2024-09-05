#pragma once

#include <QString>
#include <QTcpServer>

namespace ct::network {
   class server : QTcpServer {
      Q_OBJECT

   public:
      bool start(const QString& host);
   };
}
