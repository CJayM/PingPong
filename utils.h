#ifndef UTILS_H
#define UTILS_H

#include <QAbstractSocket>

QString socketErrorToString(QAbstractSocket::SocketError error);
QString findMyIpAddress();


#endif // UTILS_H
