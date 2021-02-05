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
    connect(&client_, &ClientSide::sgnMessage, this, &MainWindow::onClientMessage);
    connect(&server_, &ServerSide::sgnMessage, this, &MainWindow::onServerMessage);

    restoreSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
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
        client_.setConnectionParams(ui->editServerIp->text(), ui->spinClientPort->value(), ui->spinClientTimeout->value());
        client_.connectToServer();
    }
}

void MainWindow::onClientStateChanged(ClientState state)
{
    switch (state) {
    case ClientState::CONNECTED:
        ui->lblClientState->setText("Подключено");
        ui->lblClientState->setStyleSheet("background-color: rgb(123, 255, 114);");

        ui->btnClient->setText("Отключиться");
        ui->btnClient->setEnabled(true);
        break;
    case ClientState::CONNECTING:
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
    case ClientState::TIMEOUT:
        ui->lblClientState->setText("Превышен интервал ожидания");
        ui->lblClientState->setStyleSheet("background-color: rgb(255, 247, 156);");

        ui->btnClient->setText("Подключиться");
        ui->btnClient->setEnabled(true);

        if (ui->checkBox->isChecked()) {
            client_.setConnectionParams(ui->editServerIp->text(), ui->spinClientPort->value(), ui->spinClientTimeout->value());
            client_.connectToServer();
        }

        break;
    }
}

void MainWindow::onServerStateChanged(ServerState state)
{
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

void MainWindow::onClientMessage(QString msg)
{
    ui->textClientLog->append(msg);
}

void MainWindow::onServerMessage(QString msg)
{
    ui->textServerLog->append(msg);
}

void MainWindow::restoreSettings()
{
    ui->spinServerPort->setValue(settings.value("spinServerPort", 3333).toInt());
    ui->spinClientPort->setValue(settings.value("spinClientPort", 3333).toInt());
    ui->spinClientTimeout->setValue(settings.value("spinClientTimeout", 3000).toInt());
    ui->editServerIp->setText(settings.value("editServerIp", "127.0.0.1").toString());
}

void MainWindow::saveSettings()
{
   settings.setValue("spinServerPort", ui->spinServerPort->value());
   settings.setValue("spinClientPort", ui->spinClientPort->value());
   settings.setValue("spinClientTimeout", ui->spinClientTimeout->value());
   settings.setValue("editServerIp", ui->editServerIp->text());
}
