#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    isClientConnected_ = true;

    ui->lblServerState->setText("Клиент подключился");
    ui->lblServerState->setStyleSheet("background-color: rgb(123, 255, 114);");
    ui->textServerLog->append("Клиент подключился");

    toServerSocket_ = tcpServer->nextPendingConnection();

    connect(toServerSocket_, &QAbstractSocket::disconnected,
        this, &MainWindow::onDisconnectClient);
    connect(toServerSocket_, &QIODevice::readyRead, this, &MainWindow::onDataRead);
}

void MainWindow::onDisconnectClient()
{
    isClientConnected_ = false;

    ui->lblServerState->setText("Клиент отключился");
    ui->lblServerState->setStyleSheet("background-color: rgb(255, 247, 156);");
    ui->textServerLog->append("Клиент отключился");
}

void MainWindow::onDataRead()
{
    auto data = toServerSocket_->readAll();
    ui->textServerLog->append(data);
    toServerSocket_->write("ok\n");
}

void MainWindow::startServer()
{
    ui->lblServerState->setText("Запуск сервера...");
    ui->btnServer->setEnabled(false);
    serverPort_ = ui->spinServerPort->value();
    QTimer::singleShot(1, [&]() {
        tcpServer = new QTcpServer(this);
        connect(tcpServer, &QTcpServer::newConnection, this, &MainWindow::onClientConnected);

        if (!tcpServer->listen(QHostAddress::Any, serverPort_)) {
            ui->lblServerState->setText(QString("Не удалось запустить сервер: %1.")
                                            .arg(tcpServer->errorString()));
            ui->btnServer->setEnabled(true);
            return;
        }

        QString ipAddress;
        QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
        for (int i = 0; i < ipAddressesList.size(); ++i) {
            if (ipAddressesList.at(i) != QHostAddress::LocalHost && ipAddressesList.at(i).toIPv4Address()) {
                ipAddress = ipAddressesList.at(i).toString();
                break;
            }
        }
        // if we did not find one, use IPv4 localhost
        if (ipAddress.isEmpty())
            ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
        ui->textServerLog->append(QString("Сервер запущен по адресу: %1:%2")
                                      .arg(ipAddress)
                                      .arg(tcpServer->serverPort()));

        ui->btnServer->setEnabled(true);
        isServerStarted_ = true;
        ui->btnServer->setText("Остановить");

        ui->lblServerState->setText("Сервер запущен");
        ui->lblServerState->setStyleSheet("background-color: rgb(255, 247, 156);");
    });
}

void MainWindow::stopServer()
{
    tcpServer->close();

    isServerStarted_ = false;

    ui->btnServer->setEnabled(true);
    ui->btnServer->setText("Запустить сервер");
    ui->btnServer->setBackgroundRole(QPalette::ColorRole::Base);

    ui->lblServerState->setText("Сервер остановлен");
    ui->lblServerState->setStyleSheet("");

    ui->textServerLog->append("Сервер остановлен");

}
