#include <QCoreApplication>
#include <iostream>
#include "CTFMMController.h"
#include "BackTest/CTFMMBackTest.h"
#include "Utility/CConfig.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 读取配置信息，根据模式确定实盘还是回测
    std::map<std::string,std::string> mpConfig;
    HESS::CConfig TFMMConfig("config.ini");
    if(!TFMMConfig.getConfig(&mpConfig))
    {
        std::cout << "[Error] Failed to read config file." << std::endl;
        QCoreApplication::exit(-1);
    }
    QString strMode;
    if(mpConfig.find("Mode") != mpConfig.end())
        strMode = QString::fromStdString(mpConfig["Mode"]);
    else
    {
        std::cout << "[Error] Could not find config value of 'Mode'." << std::endl;
        QCoreApplication::exit(-1);
    }

    if(strMode == "normal")
        HESS::CTFMMController tfmmCon;
    else if(strMode == "backtest")
    {
        HESS::CTFMMBackTest tfBackTest;
        tfBackTest.doBackTest();
    }

    return a.exec();
}
