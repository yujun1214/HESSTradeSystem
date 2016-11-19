TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
#CONFIG -= qt
QT += sql

unix {
INCLUDEPATH += ../../
}

SOURCES += main.cpp \
    CSmartMoneyQ.cpp \
    ../../DataBase/DBInterface.cpp \
    ../../DataBase/BasicData.cpp \
    ../../Utility/CLogSys.cpp

HEADERS += \
    ../../Utility/DataStructDef.h \
    CSmartMoneyQ.h \
    ../../DataBase/DBInterface.h \
    ../../DataBase/BasicData.h \
    ../../Utility/CLogSys.h \
    ../../DataBase/StrategyData.h
