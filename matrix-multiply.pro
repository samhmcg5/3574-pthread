QT += core
QT -= gui

CONFIG += c++11

TARGET = matrix-multiply
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    matrix-multiply.cpp

HEADERS += \
    matrix-multiply.h

LIBS += -pthread
