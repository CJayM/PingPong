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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onServerBtnClick()
{
    if (isServerStarted_) {
        stopServer();
    } else {
        startServer();
    }
}

void MainWindow::onClientConnected()
{
    clientServerSocket_ = tcpServer->nextPendingConnection();
    connect(clientServerSocket_, &QAbstractSocket::disconnected,
        this, &MainWindow::onDisconnectClient);
    connect(clientServerSocket_, &QIODevice::readyRead, this, &MainWindow::onDataRead);

    setServerModeState(ServerState::HAS_CLIENT);
    QString addr = QString("%1:%2").arg(clientServerSocket_->peerAddress().toString()).arg(clientServerSocket_->peerPort());
    ui->textServerLog->append(QString("Клиент %1 подключён").arg(addr));
}

void MainWindow::onDisconnectClient()
{
    setServerModeState(ServerState::NO_CLIENT);
    ui->textServerLog->append("Клиент отключился");
}

void MainWindow::onDataRead()
{
    auto data = clientServerSocket_->readAll();
    ui->textServerLog->append(data);
    clientServerSocket_->write("ok\n");
}

void MainWindow::onClientBtnClick()
{
    if (isClientStarted_) {
        disconnectClient();
    } else {
        connectToClient();
    }
}

void MainWindow::displayError(QAbstractSocket::SocketError socketError)
{
    setClientModeState(ClientState::ERROR);
    ui->textClientLog->append(QString("Ошибка: %1").arg(socketErrorToString(socketError)));
}

void MainWindow::startServer()
{
    serverPort_ = ui->spinServerPort->value();

    setServerModeState(ServerState::STARTING);
    ui->textServerLog->append("Запуск сервера");

    QTimer::singleShot(1, [&]() {
        tcpServer = new QTcpServer(this);
        connect(tcpServer, &QTcpServer::newConnection, this, &MainWindow::onClientConnected);

        if (!tcpServer->listen(QHostAddress::Any, serverPort_)) {
            setServerModeState(ServerState::ERROR);
            ui->lblServerState->setText(QString("Не удалось запустить сервер: %1.")
                                            .arg(tcpServer->errorString()));
            return;
        }

        setServerModeState(ServerState::STARTED);

        QString ipAddress = findMyIpAddress();

        ui->textServerLog->append(QString("Сервер запущен по адресу: %1:%2")
                                      .arg(ipAddress)
                                      .arg(tcpServer->serverPort()));

    });
}

void MainWindow::stopServer()
{
    tcpServer->close();

    setServerModeState(ServerState::STOPPED);
    ui->textServerLog->append("Сервер остановлен");
}

void MainWindow::connectToClient()
{
    clientServerIp_ = ui->editServerIp->text();
    clientPort_ = ui->spinClientPort->value();

    setClientModeState(ClientState::CONNECTING);

    clientSocket_ = new QTcpSocket(this);
    connect(clientSocket_, &QTcpSocket::connected, this, &MainWindow::onConnectedToServer);
    connect(clientSocket_, &QIODevice::readyRead, this, &MainWindow::onServerDataRead);
    connect(clientSocket_, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
        this, &MainWindow::displayError);

    clientSocket_->connectToHost(clientServerIp_, clientPort_);
}

void MainWindow::onConnectedToServer()
{
    setClientModeState(ClientState::CONNECTED);

    QString addr = QString("%1:%2").arg(clientServerIp_).arg(clientPort_);
    ui->textClientLog->append(QString("Соединение с %1 успешно").arg(addr));

    clientSocket_->write("Ping\n");
}

void MainWindow::onServerDataRead()
{
    auto data = clientServerSocket_->readAll();
    ui->textClientLog->append(data);
    clientSocket_->write("pong\n");
}

void MainWindow::disconnectClient()
{
    setClientModeState(ClientState::DISCONNECTED);
    ui->textClientLog->append("Подключение разорвано");

    if (clientSocket_) {
        clientSocket_->close();
        clientSocket_->deleteLater();
        clientSocket_ = nullptr;
    }
}

void MainWindow::setClientModeState(ClientState state)
{
    switch (state) {
    case ClientState::CONNECTED:
        isClientStarted_ = true;

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
        isClientStarted_ = false;

        ui->lblClientState->setText("Отключено");
        ui->lblClientState->setStyleSheet("background-color: rgb(255, 114, 114);");

        ui->btnClient->setText("Подключиться");
        ui->btnClient->setEnabled(true);

        break;
    case ClientState::ERROR:
        isClientStarted_ = false;
        break;
    }

    ui->lblServerState->setText("Клиент подключился");
    ui->lblServerState->setStyleSheet("background-color: rgb(123, 255, 114);");
    ui->textServerLog->append("Клиент подключился");
}

void MainWindow::setServerModeState(ServerState state)
{
    switch (state) {
    case ServerState::STARTED:
        isServerStarted_ = true;

        ui->btnServer->setEnabled(true);
        ui->btnServer->setText("Остановить");

        ui->lblServerState->setText("Сервер запущен");
        ui->lblServerState->setStyleSheet("background-color: rgb(255, 247, 156);");
        break;
    case ServerState::STOPPED:
        isServerStarted_ = false;
        isClientConnected_ = false;

        ui->btnServer->setEnabled(true);
        ui->btnServer->setText("Запустить сервер");

        ui->lblServerState->setText("Сервер остановлен");
        ui->lblServerState->setStyleSheet("");
        break;
    case ServerState::STARTING:
        isServerStarted_ = false;
        ui->lblServerState->setText("Запуск сервера...");
        ui->lblServerState->setStyleSheet("background-color: rgb(114, 114, 114);");
        ui->btnServer->setEnabled(false);
        break;
    case ServerState::HAS_CLIENT:
        isClientConnected_ = true;

        ui->lblServerState->setText("Клиент подключился");
        ui->lblServerState->setStyleSheet("background-color: rgb(123, 255, 114);");
        break;
    case ServerState::NO_CLIENT:
        isClientConnected_ = true;

        ui->lblServerState->setText("Клиент отключился");
        ui->lblServerState->setStyleSheet("background-color: rgb(255, 247, 156);");
        break;
    case ServerState::ERROR:
        isServerStarted_ = false;
        isClientConnected_ = false;

        ui->btnServer->setEnabled(true);
        ui->btnServer->setText("Запустить сервер");

        ui->lblServerState->setText("Произошла ошибка");
        ui->lblServerState->setStyleSheet("background-color: rgb(255, 114, 114);");
        break;
    }
}
