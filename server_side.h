#ifndef SERVERSIDE_H
#define SERVERSIDE_H

#include <QTcpServer>
#include <QTcpSocket>

enum class ServerState {
    STOPPED,
    STARTED,
    STARTING,
    HAS_CLIENT,
    NO_CLIENT,
    ERROR,
};

class ServerSide : public QObject {
    Q_OBJECT
public:
    ServerSide();

    bool isStarted() const;
    void stop();
    void start();

    void setPort(int port);

signals:
    void sgnStateChanged(ServerState state);
    void sgnMessage(QString msg);

private slots:
    void onClientConnected();
    void onDisconnectClient();
    void onDataRead();

private:
    QTcpServer* tcpServer = nullptr;
    QTcpSocket* clientServerSocket_ = nullptr;
    bool isServerStarted_ = false;
    uint serverPort_;
    bool isClientConnected_ = true;
    QString lastServerError_;
};

#endif // SERVERSIDE_H
