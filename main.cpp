#include "mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("kb32");
    QCoreApplication::setOrganizationDomain("kb23.46");
    QCoreApplication::setApplicationName("Ping Pong");

    MainWindow w;
    w.show();
    return a.exec();
}
