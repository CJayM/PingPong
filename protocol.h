#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QDataStream>

struct MessageHeader {
    quint32 prefix = 0;
    quint16 id = 0;
    quint64 startTime = 0;
    quint32 size = 0;

    bool isCorrect() const;
    static int messageSize();
};

class MessageBody {
public:
    quint32 crc = 0;
    quint64 endTime = 0;
    quint32 postfix = 0;

    quint32 rawCrc = 0;

    MessageBody();
    MessageBody(quint32 size);

    bool isCorrect() const;
    quint32 getFullSize() const;
    quint32 getBodySize() const;

    static quint32 getFullSize(quint32 size);

private:
    quint32 size_ = 0;
};

struct Message {
    MessageHeader header;
    MessageBody body;

    Message();
    Message(MessageHeader header);
    Message(MessageHeader header, MessageBody body);

    bool isCorrect() const;
};

struct Answer {
    quint32 prefix = 0;
    quint16 id = 0;
    quint64 startTime = 0;
    quint64 endTime = 0;
    quint32 postfix = 0;

    quint64 deltaTime = 0;
    quint64 avrTime = 0;

    bool isCorrect() const;
    static int messageSize();
};

QDataStream& operator<<(QDataStream& stream, const MessageHeader& msg);
QDataStream& operator>>(QDataStream& stream, MessageHeader& msg);

QDataStream& operator<<(QDataStream& stream, const MessageBody& msg);
QDataStream& operator>>(QDataStream& stream, MessageBody& msg);

QDataStream& operator<<(QDataStream& stream, const Answer& msg);
QDataStream& operator>>(QDataStream& stream, Answer& msg);

#endif // PROTOCOL_H
