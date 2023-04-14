QT       += core

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console
CONFIG -= app_bundle

QMAKE_CXXFLAGS += -Werror=return-type

SOURCES += \
    main.cpp \
    FunctionParser.cpp

HEADERS += \
    FunctionParser.h
