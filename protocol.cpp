#include "protocol.h"
#include "utils.h"

#include <QDateTime>

constexpr quint32 MESSAGE_PREFFIX = 0xfafbfcfd;
constexpr quint32 MESSAGE_POSTFIX = 0xfcfdfeff;

QDataStream& operator<<(QDataStream& stream, const MessageHeader& msg)
{
    stream << MESSAGE_PREFFIX;
    stream << msg.id;
    stream << msg.time;
    stream << msg.size;

    StreamedAdlerCrc crc;
    std::random_device random;
    quint8 value;
    quint32 end = msg.size * 1024;
    for (quint32 i = 0; i < end; ++i) {
        value = quint8(random() % 255);
        crc.addValue(value);
        stream << value;
    }

    stream << crc.getCrc();
    return stream;
}
QDataStream& operator>>(QDataStream& stream, MessageHeader& msg)
{
    stream >>msg.prefix;
    stream >> msg.id;
    stream >> msg.time;
    stream >> msg.size;
    return stream;
}

QDataStream& operator<<(QDataStream& stream, const Answer& msg)
{
    stream << MESSAGE_PREFFIX;
    stream << msg.id;
    stream << msg.time;
    stream << MESSAGE_POSTFIX;
    return stream;
}
QDataStream& operator>>(QDataStream& stream, Answer& msg)
{
    stream >> msg.prefix;
    stream >> msg.id;
    stream >> msg.time;
    stream >> msg.postfix;

    msg.deltaTime = QDateTime::currentMSecsSinceEpoch() - msg.time;
    return stream;
}

bool Answer::isCorrect() const
{
    if ((prefix == MESSAGE_PREFFIX) && (postfix == MESSAGE_POSTFIX))
        return true;

    if (time > receivedTime)
        return false;

    return false;
}

constexpr int Answer::messageSize()
{
    return sizeof(Answer::prefix)
        + sizeof(Answer::id)
        + sizeof(Answer::time)
        + sizeof(Answer::postfix);
}

bool MessageHeader::isCorrect() const
{
    if (prefix == MESSAGE_PREFFIX)
        return true;

    return false;
}

bool MessageBody::isCorrect(quint32 originCrc) const
{
    if (originCrc != crc)
        return false;

    if (postfix != MESSAGE_POSTFIX)
        return false;

    return true;
}
