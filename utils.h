#ifndef UTILS_H
#define UTILS_H

#include <QAbstractSocket>

QString socketErrorToString(QAbstractSocket::SocketError error);
QString findMyIpAddress();

quint32 calculateAdlerCrc(QDataStream& stream, qint64 from, qint64 to);

struct StreamedAdlerCrc {
    void addValue(quint8 value);

    quint32 getCrc() const;
    void reset();

private:
    quint32 s1 = 1;
    quint32 s2 = 0;
};

#endif // UTILS_H
