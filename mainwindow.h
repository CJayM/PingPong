#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>


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

private:
    Ui::MainWindow *ui;    

    QTcpServer* tcpServer = nullptr;
    QTcpSocket* toServerSocket_ = nullptr;
    bool isServerStarted_ = false;
    uint serverPort_;
    bool isClientConnected_ = true;

    void startServer();
    void stopServer();
};
#endif // MAINWINDOW_H
