#ifndef CREGRESS_H
#define CREGRESS_H

/*计算回归参数，并向外广播计算结果*/
#include <QObject>
#include "Utility/DataStructDef.h"
#include <QMutex>

namespace HESS
{

#define MKTDATAARRAYLENGTH 40000    // 行情数组长度
#define MKTDATAUSEDLENGTH 512       // 计算回归参数所采用的行情数据长度

class CRegress:public QObject
{
    Q_OBJECT
public:
    CRegress(QString _strIndependentTicker, QString _strDependentTicker);
    ~CRegress();

//    void addRegressPair(QString strIndependentTicker,QString strDependentTicker);

private:
    // 回归参数计算结构
    TCointegrationParam m_tRegressParam;

    // 自变量代码
    QString m_strIndependentTicker;
    // 因变量代码
    QString m_strDependentTicker;

    // 自变量的乘数
    int m_nIndependentMultiple;
    // 因变量的乘数
    int m_nDependentMultiple;

    // 自变量行情数组，在堆上分配，默认长度为40000个
    double* m_pIndependentMktData;
    // 因变量行情数据，在堆上分配，默认长度为40000个
    double* m_pDependentMktData;
    // 行情数据的互斥锁
    QMutex m_MktMutex;

    // 自变量行情数组的更新位置
    int m_nIndependentMktUpdatePos;
    // 因变量行情数组的更新位置
    int m_nDependentMktUpdatePos;

    // 计算回归参数所采用的行情数据长度
//    int m_nMktDataUsedLength;

private:
    // 计算回归参数
    bool CalcRegressionParam();

public slots:
    // 更新行情，需判断行情是否一致
    void updateMktData(TDerivativeMktDataField _mktData);

signals:
    // 发送计算结果
    void CalculationDone(TCointegrationParam _param);
};

}
#endif // CREGRESS_H
