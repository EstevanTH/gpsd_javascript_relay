#-------------------------------------------------
#
# Project created by QtCreator 2017-01-31T21:15:48
#
#-------------------------------------------------

QT       += core gui network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Sécurité
equals( QMAKE_LINK, g++ ){
	QMAKE_CFLAGS_RELEASE += -fstack-protector-all
	QMAKE_CXXFLAGS_RELEASE += -fstack-protector-all
	QMAKE_LFLAGS_RELEASE += -Wl,--nxcompat -Wl,--dynamicbase,--export-all-symbols -fstack-protector-all
	contains( QMAKE_QMAKE, 64 ){
		QMAKE_LFLAGS_RELEASE += -Wl,--high-entropy-va
	}
}
equals( QMAKE_LINK, link ){
	QMAKE_LFLAGS_RELEASE += /NXCOMPAT /DYNAMICBASE
	contains( QMAKE_QMAKE, 64 ){
		QMAKE_LFLAGS_RELEASE += /HIGHENTROPYVA
	}
}

TARGET = gpsd_javascript_relay
TEMPLATE = app

QMAKE_TARGET_DESCRIPTION = "GPSD to JavaScript relay"


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
    ValidatorJavascriptFunction.cpp \
    SingleInstanceData.cpp

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
    ValidatorJavascriptFunction.h \
    compiler_options.h \
    SingleInstanceData.h

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

TRANSLATIONS = gpsd_javascript_relay_fr.ts
