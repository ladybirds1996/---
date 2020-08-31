QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Server
TEMPLATE = app


SOURCES += main.cpp\
           server.cpp \
           protocol.cpp

HEADERS  += \
            protocol.h \
            server.h

FORMS    += server.ui
