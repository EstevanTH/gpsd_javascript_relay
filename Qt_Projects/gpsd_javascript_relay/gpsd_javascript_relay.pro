#-------------------------------------------------
#
# Project created by QtCreator 2017-01-31T21:15:48
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gpsd_javascript_relay
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    HttpServer.cpp \
    GpsdClient.cpp \
    TargetTabSetupWidget.cpp \
    GpsdTabSetupWidget.cpp \
    GpsdStatusWidget.cpp \
    HttpServerClient.cpp \
    GpsdHostWidget.cpp \
    GpsdHost.cpp \
    Application.cpp \
    ValidatorHttpFilename.cpp \
    ValidatorJavascriptFunction.cpp

HEADERS  += MainWindow.h \
    HttpServer.h \
    GpsdClient.h \
    TargetTabSetupWidget.h \
    GpsdTabSetupWidget.h \
    GpsdStatusWidget.h \
    HttpServerClient.h \
    GpsdHostWidget.h \
    GpsdHost.h \
    Application.h \
    ValidatorHttpFilename.h \
    ValidatorJavascriptFunction.h

FORMS    += MainWindow.ui \
    TargetTabSetupWidget.ui \
    GpsdTabSetupWidget.ui \
    GpsdStatusWidget.ui \
    GpsdHostWidget.ui

DISTFILES += \
    Contenu.txt \
    Bugs.txt

RC_ICONS = gpsd_javascript_relay_32.ico

RESOURCES += \
    gpsd_javascript_relay.qrc

TRANSLATIONS = tp-znavigo-le-navigateur-web-des-zeros_fr.ts
