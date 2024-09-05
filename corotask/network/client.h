#pragma once

#include <vector>
#include <QString>
#include <QTcpSocket>
#include <QStringList>

namespace ct::network {
   class client : QTcpSocket {
      Q_OBJECT

   public:
      bool connect(const QString& host);
      QStringList get_workers();
   };
}
