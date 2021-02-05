#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "utils.h"
#include <QAbstractSocket>
#include <QIODevice>
#include <QNetworkInterface>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnServer, &QPushButton::clicked, this, &MainWindow::onServerBtnClick);
    connect(ui->btnClient, &QPushButton::clicked, this, &MainWindow::onClientBtnClick);
    connect(&client_, &ClientSide::sgnStateChanged, this, &MainWindow::onClientStateChanged);
    connect(&server_, &ServerSide::sgnStateChanged, this, &MainWindow::onServerStateChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onServerBtnClick()
{
    if (server_.isStarted()) {
        server_.stop();
    } else {
        server_.setPort(ui->spinServerPort->value());
        server_.start();
    }
}

void MainWindow::onClientBtnClick()
{
    if (client_.isStarted()) {
        client_.disconnectFromServer();
    } else {
        client_.setConnectionParams(ui->editServerIp->text(), ui->spinClientPort->value());
        client_.connectToServer();
    }
}

void MainWindow::onClientStateChanged(ClientState state, QString msg)
{
    ui->textClientLog->append(msg);

    switch (state) {
    case ClientState::CONNECTED:
        ui->lblClientState->setText("Подключено");
        ui->lblClientState->setStyleSheet("background-color: rgb(123, 255, 114);");

        ui->btnClient->setText("Отключиться");
        ui->btnClient->setEnabled(true);
        break;
    case ClientState::CONNECTING:
        ui->textClientLog->append("Поключение к ");
        ui->lblClientState->setText("Подключение к удалённому серверу...");

        ui->btnClient->setText("Подключение...");
        ui->btnClient->setEnabled(false);
        break;
    case ClientState::DISCONNECTED:
        ui->lblClientState->setText("Отключено");
        ui->lblClientState->setStyleSheet("background-color: rgb(255, 114, 114);");

        ui->btnClient->setText("Подключиться");
        ui->btnClient->setEnabled(true);

        break;
    case ClientState::ERROR:
        ui->lblClientState->setText("Ошибка");
        ui->lblClientState->setStyleSheet("background-color: rgb(255, 114, 114);");

        ui->btnClient->setText("Подключиться");
        ui->btnClient->setEnabled(true);
        break;
    }
}

void MainWindow::onServerStateChanged(ServerState state, QString msg)
{
    ui->textServerLog->append(msg);

    switch (state) {
    case ServerState::STARTED:
        ui->btnServer->setEnabled(true);
        ui->btnServer->setText("Остановить");

        ui->lblServerState->setText("Сервер запущен");
        ui->lblServerState->setStyleSheet("background-color: rgb(255, 247, 156);");
        break;
    case ServerState::STOPPED:
        ui->btnServer->setEnabled(true);
        ui->btnServer->setText("Запустить сервер");

        ui->lblServerState->setText("Сервер остановлен");
        ui->lblServerState->setStyleSheet("");
        break;
    case ServerState::STARTING:
        ui->lblServerState->setText("Запуск сервера...");
        ui->lblServerState->setStyleSheet("background-color: rgb(114, 114, 114);");
        ui->btnServer->setEnabled(false);
        break;
    case ServerState::HAS_CLIENT:
        ui->lblServerState->setText("Клиент подключился");
        ui->lblServerState->setStyleSheet("background-color: rgb(123, 255, 114);");
        break;
    case ServerState::NO_CLIENT:
        ui->lblServerState->setText("Клиент отключился");
        ui->lblServerState->setStyleSheet("background-color: rgb(255, 247, 156);");
        break;
    case ServerState::ERROR:
        ui->btnServer->setEnabled(true);
        ui->btnServer->setText("Запустить сервер");

        ui->lblServerState->setText("Произошла ошибка");
        ui->lblServerState->setStyleSheet("background-color: rgb(255, 114, 114);");
        break;
    }
}
