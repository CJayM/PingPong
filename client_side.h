#ifndef CLIENTSIDE_H
#define CLIENTSIDE_H

#include <QTcpSocket>

enum class ClientState{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR,
};

class ClientSide: public QObject {
    Q_OBJECT
public:
    ClientSide();

    bool isStarted() const;

    void disconnectFromServer();
    void connectToServer();

    void setConnectionParams(QString ip, int port);

signals:
    void sgnStateChanged(ClientState state, QString msg);

private slots:
    void displayError(QAbstractSocket::SocketError socketError);
    void onConnectedToServer();
    void onServerDataRead();

private:
    QTcpSocket* clientSocket_ = nullptr;
    bool isClientStarted_ = false;
    QString clientServerIp_ = "127.0.0.1";
    uint clientPort_ = 3333;
    QString lastClientError_;
};

#endif // CLIENTSIDE_H
