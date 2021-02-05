#include "client_side.h"
#include "utils.h"

#include <QDateTime>

ClientSide::ClientSide()
{
    connect(&timeoutTimer_, &QTimer::timeout, this, &ClientSide::onTimeOut);
    connect(&messagesTimer_, &QTimer::timeout, this, &ClientSide::onMessageTimeOut);
}

bool ClientSide::isStarted() const
{
    return isClientStarted_;
}

void ClientSide::disconnectFromServer()
{
    isClientStarted_ = false;
    emit sgnMessage("Подключение разорвано");
    emit sgnStateChanged(ClientState::DISCONNECTED);

    messagesTimer_.stop();

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
    emit sgnMessage(msg);
    emit sgnStateChanged(ClientState::CONNECTING);

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
    if (timeoutWaiting_ == true) {
        timeoutWaiting_ = false;
        return;
    }

    auto msg = QString("Ошибка: %1").arg(socketErrorToString(socketError));
    emit sgnMessage(msg);
    emit sgnStateChanged(ClientState::ERROR);
    messagesTimer_.stop();
}

void ClientSide::onConnectedToServer()
{
    isClientStarted_ = true;
    timeoutWaiting_ = false;

    QString msg = QString("Соединение с %1:%2 успешно").arg(clientServerIp_).arg(clientPort_);
    emit sgnMessage(msg);
    emit sgnStateChanged(ClientState::CONNECTED);

    messagesTimer_.start(1000);
    onMessageTimeOut();
}

void ClientSide::onServerDataRead()
{
    auto now = QDateTime::currentMSecsSinceEpoch();
    auto data = clientSocket_->readAll();
    bool success;
    qint64 answer = data.toLongLong(&success);
    int delta = 0;
    if (hashedTime_.contains(answer) == false){
        emit sgnMessage("Принято неизвестное значение");
        return;
    }

    delta = now - hashedTime_[answer];
    hashedTime_.remove(answer);
    emit sgnMessage(QString("Получен ответ за %1 ms").arg(delta));
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
    emit sgnMessage("Превышен интервал ожидания");
    emit sgnStateChanged(ClientState::TIMEOUT);
}

void ClientSide::onMessageTimeOut()
{
    auto value = ++increment_;
    hashedTime_[value] = QDateTime::currentMSecsSinceEpoch();
    clientSocket_->write(QString::number(value).toLatin1());
}
