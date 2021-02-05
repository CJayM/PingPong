#include "client_side.h"
#include "utils.h"

ClientSide::ClientSide()
{
}

bool ClientSide::isStarted() const
{
    return isClientStarted_;
}

void ClientSide::disconnectFromServer()
{
    isClientStarted_ = false;
    emit sgnStateChanged(ClientState::DISCONNECTED, "Подключение разорвано");

    if (clientSocket_) {
        clientSocket_->close();
        clientSocket_->deleteLater();
        clientSocket_ = nullptr;
    }
}

void ClientSide::connectToServer()
{
    isClientStarted_ = false;
    emit sgnStateChanged(ClientState::CONNECTING, "Подключение...");

    clientSocket_ = new QTcpSocket(this);
    connect(clientSocket_, &QTcpSocket::connected, this, &ClientSide::onConnectedToServer);
    connect(clientSocket_, &QIODevice::readyRead, this, &ClientSide::onServerDataRead);
    connect(clientSocket_, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
        this, &ClientSide::displayError);

    clientSocket_->connectToHost(clientServerIp_, clientPort_);
}

void ClientSide::setConnectionParams(QString ip, int port)
{
    clientServerIp_ = ip;
    clientPort_ = port;
}

void ClientSide::displayError(QAbstractSocket::SocketError socketError)
{
    isClientStarted_ = false;
    auto msg = QString("Ошибка: %1").arg(socketErrorToString(socketError));
    emit sgnStateChanged(ClientState::ERROR, msg);
}

void ClientSide::onConnectedToServer()
{
    isClientStarted_ = true;

    QString msg = QString("Соединение с %1:%2 успешно").arg(clientServerIp_).arg(clientPort_);
    emit sgnStateChanged(ClientState::CONNECTED, msg);

    clientSocket_->write("Ping\n");
}

void ClientSide::onServerDataRead()
{
    auto data = clientSocket_->readAll();
//    ui->textClientLog->append(data);
    clientSocket_->write("pong\n");
}
