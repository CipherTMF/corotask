#include "server.h"
#include <QUrl>
using namespace ct::network;

bool server::start(const QString& host) {
   const auto url = QUrl(host);
   return listen(QHostAddress(url.host()), url.port());
}
