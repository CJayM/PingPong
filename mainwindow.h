#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "client_side.h"
#include "server_side.h"

#include <QAbstractSocket>
#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onServerBtnClick();

    void onClientBtnClick();
    void onClientStateChanged(ClientState state);
    void onServerStateChanged(ServerState state);
    void onClientMessage(QString msg);
    void onServerMessage(QString msg);

private:
    Ui::MainWindow* ui;

    ServerSide server_;
    ClientSide client_;
};
#endif // MAINWINDOW_H
