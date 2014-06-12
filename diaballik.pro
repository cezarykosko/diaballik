#-------------------------------------------------
#
# Project created by QtCreator 2013-06-15T15:58:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = diaballik
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    tile.cpp \
    metrics.cpp \
    pawn.cpp \
    player.cpp

HEADERS  += mainwindow.h \
    tile.h \
    metrics.h \
    pawn.h \
    player.h

FORMS    += mainwindow.ui
