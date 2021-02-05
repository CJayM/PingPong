#ifndef CLIENTSIDE_H
#define CLIENTSIDE_H

#include <QTcpSocket>
#include <QTimer>

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

    void setConnectionParams(QString ip, int port, int timeout, int mesSize);

signals:
    void sgnStateChanged(ClientState state);
    void sgnMessage(QString msg);

private slots:
    void displayError(QAbstractSocket::SocketError socketError);
    void onConnectedToServer();
    void onServerDataRead();
    void onTimeOut();
    void onMessageTimeOut();
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

    QTimer messagesTimer_;
    int increment_ = 0;
    QHash<int, qint64> hashedTime_;
};

#endif // CLIENTSIDE_H
