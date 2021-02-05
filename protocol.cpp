#include "protocol.h"
#include "utils.h"

#include <QDateTime>

constexpr quint32 MESSAGE_PREFFIX = 0xfafbfcfd;
constexpr quint32 MESSAGE_POSTFIX = 0xfcfdfeff;
constexpr quint32 BYTES_PER_KILO = 1024;

QDataStream& operator<<(QDataStream& stream, const MessageHeader& msg)
{
    stream << MESSAGE_PREFFIX;
    stream << msg.id;
    stream << QDateTime::currentMSecsSinceEpoch();
    stream << msg.size;
    return stream;
}
QDataStream& operator>>(QDataStream& stream, MessageHeader& msg)
{
    stream >> msg.prefix;
    stream >> msg.id;
    stream >> msg.startTime;
    stream >> msg.size;
    return stream;
}

QDataStream& operator<<(QDataStream& stream, const MessageBody& msg)
{
    StreamedAdlerCrc crc;
    std::random_device random;
    quint8 value;
    quint32 end = msg.getBodySize();
    for (quint32 i = 0; i < end; ++i) {
        value = quint8(random() % 255);
        crc.addValue(value);
        stream << value;
    }

    stream << crc.getCrc();
    stream << QDateTime::currentMSecsSinceEpoch();
    stream << MESSAGE_POSTFIX;

    return stream;
}
QDataStream& operator>>(QDataStream& stream, MessageBody& msg)
{
    StreamedAdlerCrc crc;
    quint8 value;
    quint32 end = msg.getBodySize();
    for (quint32 i = 0; i < end; ++i) {
        stream >> value;
        crc.addValue(value);
    }

    stream >> msg.crc;
    stream >> msg.endTime;
    stream >> msg.postfix;
    msg.rawCrc = crc.getCrc();

    return stream;
}

QDataStream& operator<<(QDataStream& stream, const Answer& msg)
{
    stream << MESSAGE_PREFFIX;
    stream << msg.id;
    stream << msg.startTime;
    stream << msg.endTime;
    stream << MESSAGE_POSTFIX;
    return stream;
}
QDataStream& operator>>(QDataStream& stream, Answer& msg)
{
    auto now = QDateTime::currentMSecsSinceEpoch();

    stream >> msg.prefix;
    stream >> msg.id;
    stream >> msg.startTime;
    stream >> msg.endTime;
    stream >> msg.postfix;

    auto deltaStart = now - msg.startTime;
    auto deltaEnd = now - msg.endTime;
    msg.deltaTime = deltaEnd;
    msg.avrTime = (deltaStart + deltaEnd) / 2;

    return stream;
}

bool Answer::isCorrect() const
{
    if ((prefix == MESSAGE_PREFFIX) && (postfix == MESSAGE_POSTFIX))
        return true;

    if (startTime > endTime)
        return false;

    return false;
}

int Answer::messageSize()
{
    return sizeof(Answer::prefix)
        + sizeof(Answer::id)
        + sizeof(Answer::startTime)
        + sizeof(Answer::endTime)
        + sizeof(Answer::postfix);
}

bool MessageHeader::isCorrect() const
{
    if (prefix == MESSAGE_PREFFIX)
        return true;

    return false;
}

int MessageHeader::messageSize()
{
    return sizeof(MessageHeader::prefix)
        + sizeof(MessageHeader::id)
        + sizeof(MessageHeader::startTime)
        + sizeof(MessageHeader::size);
}

MessageBody::MessageBody()
    : MessageBody(0)
{
}

MessageBody::MessageBody(quint32 size)
    : size_(size)
{
}

bool MessageBody::isCorrect() const
{
    if (rawCrc != crc)
        return false;

    if (postfix != MESSAGE_POSTFIX)
        return false;

    return true;
}

quint32 MessageBody::getFullSize() const
{
    return getBodySize()
        + sizeof(MessageBody::crc)
        + sizeof(MessageBody::endTime)
        + sizeof(MessageBody::postfix);
}

quint32 MessageBody::getBodySize() const
{
    return size_ * BYTES_PER_KILO;
}

quint32 MessageBody::getFullSize(quint32 size)
{
    return size * BYTES_PER_KILO
        + sizeof(MessageBody::crc)
        + sizeof(MessageBody::endTime)
        + sizeof(MessageBody::postfix);
}

Message::Message()
    : header({})
    , body()
{
}

Message::Message(MessageHeader pHeader)
    : header(pHeader)
    , body(header.size)
{
}

Message::Message(MessageHeader pHeader, MessageBody pBody)
    : header(pHeader)
    , body(pBody)
{
}

bool Message::isCorrect() const
{
    if (header.startTime > body.endTime)
        return false;

    return true;
}
