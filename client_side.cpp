#include "client_side.h"

#include <QDateTime>
#include <QDebug>

#include "utils.h"
#include "protocol.h"
#include "string_templates.h"


ClientSide::ClientSide()
{
    connect(&timeoutTimer_, &QTimer::timeout, this, &ClientSide::onTimeOut);
    connect(&answerTimer_, &QTimer::timeout, this, &ClientSide::onAnwerTimeOut);
}

bool ClientSide::isStarted() const
{
    return isClientStarted_;
}

void ClientSide::disconnectFromServer()
{
    onDisconnected();
}

void ClientSide::connectToServer()
{
    isClientStarted_ = false;
    QString msg = templates::MSG_CONNECTING_TO.arg(clientServerIp_).arg(clientPort_);
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

void ClientSide::setConnectionParams(QString ip, int port, int timeout, int pMesSize)
{
    timeoutDuration_ = timeout;
    clientServerIp_ = ip;
    clientPort_ = port;
    mesSize_ = pMesSize;
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
}

void ClientSide::onConnectedToServer()
{
    timeoutTimer_.stop();

    write_.setDevice(clientSocket_);
    write_.setVersion(QDataStream::Qt_5_0);
    write_.setByteOrder(QDataStream::LittleEndian);

    isClientStarted_ = true;
    timeoutWaiting_ = false;

    connect(clientSocket_, &QAbstractSocket::disconnected, this, &ClientSide::onDisconnected);

    QString msg = templates::MSG_CONNECTION_SUCCESSFUL.arg(clientServerIp_).arg(clientPort_);
    emit sgnMessage(msg);
    emit sgnStateChanged(ClientState::CONNECTED);

    sendMessage();
}

void ClientSide::onServerDataRead()
{
    answerTimer_.stop();

    {
        QMutexLocker locker(&mutex_);
        readBuffer_.append(clientSocket_->readAll());
    }

    auto answers = parseAnswers();
    proccessAnswers(answers);    
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

void ClientSide::onAnwerTimeOut()
{
    answerTimer_.stop();
    emit sgnMessage("Превышен интервал ожидания ответа");
    sendMessage();
}

void ClientSide::sendMessage()
{
    if ((clientSocket_ == nullptr) || (clientSocket_->isOpen() == false))
        return;


    MessageHeader header;
    header.id = ++increment_;
    header.size = mesSize_;
    header.startTime = QDateTime::currentMSecsSinceEpoch();

    MessageBody body{ header.size };

    emit sgnMessage(templates::MSG_SENDING.arg(header.id).arg(header.size));
    write_ << header;
    write_ << body;
    clientSocket_->flush();
    answerTimer_.start(answerTimeoutDuration_);
}

void ClientSide::onDisconnected()
{
    answerTimer_.stop();
    isClientStarted_ = false;
    emit sgnMessage("Подключение разорвано");
    emit sgnStateChanged(ClientState::DISCONNECTED);

    if (clientSocket_) {
        clientSocket_->close();
        clientSocket_->deleteLater();
        clientSocket_ = nullptr;
    }
}

QVector<Answer> ClientSide::parseAnswers()
{
    QVector<Answer> answers;
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
        if (available < Answer::messageSize())
            break;

        auto pos = stream.device()->pos();
        Answer answer;
        stream >> answer;
        if (answer.isCorrect() == false) {
            skipped += 1;
            stream.device()->seek(pos + 1);
            eated = stream.device()->pos();
            continue;
        }

        if (skipped > 0) {
            emit sgnMessage(templates::MSG_CANT_PARSE.arg(skipped));
            skipped = 0;
        }

        answers << answer;
        eated = stream.device()->pos();
    }

    if (eated > 0) {
        QMutexLocker lock(&mutex_);
        readBuffer_.remove(0, eated);
    }

    return answers;
}

void ClientSide::proccessAnswers(QVector<Answer> answers)
{
    if (answers.empty())
        return;

    for (const auto& answer : answers) {
        emit sgnMessage(templates::MSG_RECEIVE_ANSWER.arg(answer.id).arg(answer.avrTime));
    }

    QTimer::singleShot(1000,[&](){
        sendMessage();
    });
}
