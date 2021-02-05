QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += network

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    client_side.cpp \
    main.cpp \
    mainwindow.cpp \
    server_side.cpp \
    utils.cpp

HEADERS += \
    client_side.h \
    mainwindow.h \
    server_side.h \
    utils.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
