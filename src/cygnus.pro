QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cygnus
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp
SOURCES += MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui
