#include "client_side.h"
#include "utils.h"

ClientSide::ClientSide()
{
    connect(&timeoutTimer_, &QTimer::timeout, this, &ClientSide::onTimeOut);
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
    QString msg = QString("Подключение к %1:%2 ...").arg(clientServerIp_).arg(clientPort_);
    emit sgnStateChanged(ClientState::CONNECTING, msg);

    clientSocket_ = new QTcpSocket(this);
    connect(clientSocket_, &QTcpSocket::connected, this, &ClientSide::onConnectedToServer);
    connect(clientSocket_, &QIODevice::readyRead, this, &ClientSide::onServerDataRead);
    connect(clientSocket_, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
        this, &ClientSide::displayError);

    timeoutWaiting_ = true;
    clientSocket_->connectToHost(clientServerIp_, clientPort_);
    timeoutTimer_.start(timeoutDuration_);
}

void ClientSide::setConnectionParams(QString ip, int port, int timeout)
{
    timeoutDuration_ = timeout;
    clientServerIp_ = ip;
    clientPort_ = port;
}

void ClientSide::displayError(QAbstractSocket::SocketError socketError)
{
    isClientStarted_ = false;
    if (timeoutWaiting_ == true){
        timeoutWaiting_ = false;
        return;
    }

    auto msg = QString("Ошибка: %1").arg(socketErrorToString(socketError));
    emit sgnStateChanged(ClientState::ERROR, msg);
}

void ClientSide::onConnectedToServer()
{
    isClientStarted_ = true;
    timeoutWaiting_ = false;

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

void ClientSide::onTimeOut()
{
    timeoutTimer_.stop();
    timeoutWaiting_ = false;

    if (clientSocket_) {
        if (clientSocket_->state() == QAbstractSocket::ConnectedState)
            return;

        clientSocket_->close();
        clientSocket_->deleteLater();
        clientSocket_ = nullptr;
    };

    isClientStarted_ = false;
    emit sgnStateChanged(ClientState::TIMEOUT, "Превышен интервал ожидания");
}
