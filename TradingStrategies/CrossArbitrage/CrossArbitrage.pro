#-------------------------------------------------
#
# Project created by QtCreator 2016-04-11T17:51:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CrossArbitrage
TEMPLATE = app

unix {
INCLUDEPATH += /Users/davidyujun/share/HESSTradeSystem/
}

win32 {
INCLUDEPATH += D:/share/HESSTradeSystem
}

CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        MainWindow.cpp \
    ../../MarketData/CMktDataBuffer.cpp \
    ../../Utility/DataStructDef.cpp \
    ../../Utility/CConfig.cpp \
    ../../MarketData/CCTPMdSpi.cpp \
    CCrossArbitrage.cpp \
    CRegress.cpp \
    ../../Utility/CRequestID.cpp \
    ../../Utility/COrderRef.cpp

HEADERS  += MainWindow.h \
    ../../MarketData/CMktDataBuffer.h \
    ../../Utility/DataStructDef.h \
    ../../MarketData/API/CTP/ThostFtdcMdApi.h \
    ../../Utility/ThostFtdcUserApiDataType.h \
    ../../Utility/ThostFtdcUserApiStruct.h \
    ../../MarketData/CCTPMdSpi.h \
    ../../Utility/CConfig.h \
    ../../TradeModule/CTP/ThostFtdcTraderApi.h \
    ../../Utility/cstr.h \
    CCrossArbitrage.h \
    ../../Utility/CRequestID.h \
    CRegress.h \
    ../../Utility/COrderRef.h
