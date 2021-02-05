#include "server_side.h"

#include "utils.h"
#include <QTcpServer>
#include <QTimer>

const static QString STR_CANT_START_SERVER = "Не удалось запустить сервер: %1.";
const static QString STR_SERVER_STARTED = "Сервер запущен по адресу: %1:%2";
const static QString STR_CRC_WRONG = "Не сошлась контрольная сумма у сообщения #%1";
const static QString STR_CANT_PARSE = "Не получилось разобрать %1 байт";
const static QString STR_CLIENT_CONNECTED = "Клиент %1:%2 подключён";

ServerSide::ServerSide()
{
}

bool ServerSide::isStarted() const
{
    return isServerStarted_;
}

void ServerSide::stop()
{
    tcpServer->close();
    if (clientServerSocket_ != nullptr) {
        clientServerSocket_->close();
        clientServerSocket_->deleteLater();
        clientServerSocket_ = nullptr;
    }

    isServerStarted_ = false;
    isClientConnected_ = false;

    emit sgnMessage("Сервер остановлен");
    emit sgnStateChanged(ServerState::STOPPED);
}

void ServerSide::start()
{
    isServerStarted_ = false;
    emit sgnMessage("Запуск сервера");
    emit sgnStateChanged(ServerState::STARTING);

    QTimer::singleShot(1, [&]() {
        tcpServer = new QTcpServer(this);
        connect(tcpServer, &QTcpServer::newConnection, this, &ServerSide::onClientConnected);

        if (!tcpServer->listen(QHostAddress::Any, serverPort_)) {
            isServerStarted_ = false;
            isClientConnected_ = false;

            auto msg = STR_CANT_START_SERVER.arg(tcpServer->errorString());
            emit sgnMessage(msg);
            emit sgnStateChanged(ServerState::ERROR);
            return;
        }

        isServerStarted_ = true;
        QString ipAddress = findMyIpAddress();
        auto msg = STR_SERVER_STARTED.arg(ipAddress)
                       .arg(tcpServer->serverPort());
        emit sgnMessage(msg);
        emit sgnStateChanged(ServerState::STARTED);

    });
}

void ServerSide::setPort(int port)
{
    serverPort_ = port;
}

void ServerSide::onClientConnected()
{
    clientServerSocket_ = tcpServer->nextPendingConnection();
    connect(clientServerSocket_, &QAbstractSocket::disconnected,
        this, &ServerSide::onDisconnectClient);
    connect(clientServerSocket_, &QIODevice::readyRead, this, &ServerSide::onDataRead);

    isClientConnected_ = true;
    QString msg = STR_CLIENT_CONNECTED.arg(clientServerSocket_->peerAddress().toString()).arg(clientServerSocket_->peerPort());
    emit sgnMessage(msg);
    emit sgnStateChanged(ServerState::HAS_CLIENT);
}

void ServerSide::onDisconnectClient()
{
    isClientConnected_ = true;
    emit sgnMessage("Клиент отключился");
    emit sgnStateChanged(ServerState::NO_CLIENT);
}

void ServerSide::onDataRead()
{
    {
        QMutexLocker locker(&mutex_);
        readBuffer_.append(clientServerSocket_->readAll());
    }

    auto messages = parseMessages();
    proccessMessages(messages);
}

QVector<Message> ServerSide::parseMessages()
{
    QVector<Message> messages;
    QByteArray data;
    {
        QMutexLocker lock(&mutex_);
        data = QByteArray(readBuffer_);
    }

    QDataStream stream(&data, QIODevice::OpenModeFlag::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::LittleEndian);

    qint64 eated = 0;
    quint32 skipped = 0;
    forever
    {
        auto available = stream.device()->bytesAvailable();
        if (available < MessageHeader::messageSize())
            break;

        auto pos = stream.device()->pos();

        MessageHeader header;
        stream >> header;
        if (header.isCorrect() == false) {
            skipped += 1;
            stream.device()->seek(pos + 1);
            eated = stream.device()->pos();
            continue;
        }

        auto bytesNeeded = MessageBody::getFullSize(header.size);
        auto tailSize = available - stream.device()->pos();
        if (tailSize < bytesNeeded)
            break;

        if (skipped > 0) {
            emit sgnMessage(STR_CANT_PARSE.arg(skipped));
            skipped = 0;
        }

        MessageBody body(header.size);
        stream >> body;
        eated = stream.device()->pos();
        if (body.isCorrect() == false) {
            emit sgnMessage(STR_CRC_WRONG.arg(header.id));
            continue;
        }

        Message message(header, body);
        messages << message;
    }

    if (eated > 0) {
        QMutexLocker lock(&mutex_);
        readBuffer_.remove(0, eated);
    }

    return messages;
}

void ServerSide::proccessMessages(QVector<Message> messages)
{
    if ((clientServerSocket_ == nullptr) | (clientServerSocket_->isOpen() == false))
        return;

    QDataStream stream(clientServerSocket_);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::LittleEndian);

    for(const auto& msg: messages){
        Answer answer;
        answer.id = msg.header.id;
        answer.startTime = msg.header.startTime;
        answer.endTime = msg.body.endTime;

        stream << answer;
    }

    clientServerSocket_->flush();
}
