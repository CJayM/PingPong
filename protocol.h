#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QDataStream>

struct MessageHeader {
    quint32 prefix = 0;
    quint16 id = 0;
    quint64 time = 0;
    quint32 size = 0;

    bool isCorrect() const;
};

struct MessageBody{
    quint32 crc = 0;
    quint32 postfix = 0;

    bool isCorrect(quint32 originCrc) const;
};

struct Answer {
    quint32 prefix = 0;
    quint16 id = 0;
    quint64 time = 0;
    quint32 postfix = 0;

    quint64 receivedTime = 0;
    quint64 deltaTime = 0;

    bool isCorrect() const;
    static constexpr int messageSize();
};

QDataStream& operator<<(QDataStream& stream, const MessageHeader& msg);
QDataStream& operator>>(QDataStream& stream, MessageHeader& msg);

QDataStream& operator<<(QDataStream& stream, const Answer& msg);
QDataStream& operator>>(QDataStream& stream, Answer& msg);

#endif // PROTOCOL_H
