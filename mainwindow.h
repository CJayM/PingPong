#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAbstractSocket>
#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>


enum class ServerState{
    STOPPED,
    STARTED,
    STARTING,
    HAS_CLIENT,
    NO_CLIENT,
    ERROR,
};

enum class ClientState{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR,
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onServerBtnClick();
    void onClientConnected();
    void onDisconnectClient();
    void onDataRead();

    // client
    void onClientBtnClick();
    void displayError(QAbstractSocket::SocketError socketError);
    void onConnectedToServer();
    void onServerDataRead();

private:
    Ui::MainWindow *ui;    

    QTcpServer* tcpServer = nullptr;
    QTcpSocket* clientServerSocket_ = nullptr;
    bool isServerStarted_ = false;
    uint serverPort_;
    bool isClientConnected_ = true;
    QString lastServerError_;

    // client
    QTcpSocket* clientSocket_ = nullptr;
    bool isClientStarted_ = false;
    QString clientServerIp_ = "127.0.0.1";
    uint clientPort_ = 3333;
    QString lastClientError_;

    void startServer();
    void stopServer();

    void connectToClient();
    void disconnectClient();

    void setClientModeState(ClientState state);
    void setServerModeState(ServerState state);
};
#endif // MAINWINDOW_H
