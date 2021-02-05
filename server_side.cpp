#include "server_side.h"

#include "utils.h"
#include <QTcpServer>
#include <QTimer>

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

            auto msg = QString("Не удалось запустить сервер: %1.")
                           .arg(tcpServer->errorString());
            emit sgnMessage(msg);
            emit sgnStateChanged(ServerState::ERROR);
            return;
        }

        isServerStarted_ = true;
        QString ipAddress = findMyIpAddress();
        auto msg = QString("Сервер запущен по адресу: %1:%2")
                       .arg(ipAddress)
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
    QString msg = QString("Клиент %1:%2 подключён").arg(clientServerSocket_->peerAddress().toString()).arg(clientServerSocket_->peerPort());
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
    auto data = clientServerSocket_->readAll();
    clientServerSocket_->write(data);
}
