#ifndef CLIENTSIDE_H
#define CLIENTSIDE_H

#include "protocol.h"

#include <QDataStream>
#include <QMutex>
#include <QTcpSocket>
#include <QTimer>
#include <QVector>

enum class ClientState{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR,
    TIMEOUT,
};

class ClientSide: public QObject {
    Q_OBJECT
public:
    ClientSide();

    bool isStarted() const;

    void disconnectFromServer();
    void connectToServer();

    void setConnectionParams(QString ip, int port, int timeout, int mesSize, int answerTimeout);

signals:
    void sgnStateChanged(ClientState state);
    void sgnMessage(QString msg);

private slots:
    void displayError(QAbstractSocket::SocketError socketError);
    void onConnectedToServer();
    void onServerDataRead();
    void onTimeOut();
    void onAnwerTimeOut();
    void sendMessage();
    void onDisconnected();

private:
    QTcpSocket* clientSocket_ = nullptr;
    bool isClientStarted_ = false;
    QString clientServerIp_ = "127.0.0.1";
    uint clientPort_ = 3333;
    QString lastClientError_;

    QTimer timeoutTimer_;
    bool timeoutWaiting_ = false;
    int timeoutDuration_ = 3000;    
    int mesSize_ = 1;

    int increment_ = 0;

    int answerTimeoutDuration_ = 3000;
    QTimer answerTimer_;

    QDataStream write_;
    QDataStream read_;
    QByteArray readBuffer_;
    QMutex mutex_;

    QVector<Answer> parseAnswers();
    void proccessAnswers(QVector<Answer>);
};

#endif // CLIENTSIDE_H
