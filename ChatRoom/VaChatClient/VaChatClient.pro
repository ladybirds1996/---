#-------------------------------------------------
#
# Project created by QtCreator 2017-11-02T10:29:57
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VaChatClient
TEMPLATE = app


SOURCES += main.cpp\
        register.cpp \
    mainchat.cpp \
    onechat.cpp \
    network.cpp \
    protocol.cpp

HEADERS  += register.h \
    mainchat.h \
    onechat.h \
    network.h \
    protocol.h

FORMS    += register.ui \
    mainchat.ui \
    onechat.ui
