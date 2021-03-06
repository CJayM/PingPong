QT       += core
QT       +=gui
QT       +=network
QT       +=widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    client_side.cpp \
    main.cpp \
    mainwindow.cpp \
    protocol.cpp \
    server_side.cpp \
    string_templates.cpp \
    utils.cpp

HEADERS += \
    client_side.h \
    mainwindow.h \
    protocol.h \
    server_side.h \
    string_templates.h \
    utils.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
