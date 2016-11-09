QT += core
QT -= gui

CONFIG += c++11

TARGET = TFMarketMaker
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../

SOURCES += main.cpp \
    ../../MarketData/CMktDataBuffer.cpp \
    ../../Utility/DataStructDef.cpp \
    ../../MarketData/CCTPMdSpi.cpp \
    CMMDispatcher.cpp \
    CTFMarketMaker.cpp \
    ../../Utility/COrderRef.cpp \
    ../../Utility/CRequestID.cpp \
    ../../TradeModule/CCTPTraderSpi.cpp \
    ../../Utility/CTimeController.cpp \
    ../../Utility/CConfig.cpp \
    ../../Utility/CLogSys.cpp \
    CTFMMController.cpp \
    ../../BackTest/CMktDistributorApi.cpp \
    ../../BackTest/CTradeMatchApi.cpp

HEADERS += \
    CTFMarketMaker.h \
    CMMDispatcher.h \
    ../../MarketData/CMktDataBuffer.h \
    ../../Utility/DataStructDef.h \
    ../../MarketData/CCTPMdSpi.h \
    ../../Utility/cstr.h \
    ../../Utility/COrderRef.h \
    ../../Utility/CRequestID.h \
    ../../TradeModule/CCTPTraderSpi.h \
    ../../Utility/CTimeController.h \
    ../../Utility/CConfig.h \
    ../../Utility/CLogSys.h \
    CTFMMController.h \
    ../../CTP/inc/ThostFtdcMdApi.h \
    ../../CTP/inc/ThostFtdcTraderApi.h \
    ../../CTP/inc/ThostFtdcUserApiStruct.h \
    ../../Utility/Utilities.h \
    ../../CTP/inc/thostftdcuserapidatatype.h \
    ../../BackTest/CMktDistributorApi.h \
    ../../BackTest/CTradeMatchApi.h

LIBS += -LD:/share/HESSTradeSystem/CTP/lib -lthosttraderapi
LIBS += -LD:/share/HESSTradeSystem/CTP/lib -lthostmduserapi
