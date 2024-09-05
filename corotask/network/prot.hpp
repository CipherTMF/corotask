#pragma once
#include <QString>

using namespace Qt::Literals::StringLiterals;

namespace ct::network {
   static inline auto WORKER_REQUEST = QStringLiteral("ws").toUtf8();
}
