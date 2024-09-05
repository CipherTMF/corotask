#include "client.h"
#include <QUrl>
#include "prot.hpp"
using namespace ct::network;

bool client::connect(const QString& host) {
   const auto url = QUrl(host);
   return QTcpSocket::bind(QHostAddress(url.host()), url.port());
}

QStringList client::get_workers() {
   write(WORKER_REQUEST);
   waitForReadyRead();
   return QString(readAll())
         .split(",", Qt::SkipEmptyParts);
}
