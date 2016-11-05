#include "CRegress.h"
#include <algorithm>
#include <cmath>
#include <QMutexLocker>

namespace HESS
{

CRegress::CRegress(QString _strIndependentTicker, QString _strDependentTicker):
                   m_strIndependentTicker(_strIndependentTicker),m_strDependentTicker(_strDependentTicker)
{
    // 在堆上分配行情数组的内存
    m_pIndependentMktData = new double[MKTDATAARRAYLENGTH];
    m_pDependentMktData = new double[MKTDATAARRAYLENGTH];

    // 数组更新位置初始化为0
    m_nIndependentMktUpdatePos = 0;
    m_nDependentMktUpdatePos = 0;

}// contructor

CRegress::~CRegress()
{
    delete[] m_pIndependentMktData;
    delete[] m_pDependentMktData;
}

void CRegress::updateMktData(TDerivativeMktDataField _mktData)
{
    QMutexLocker locker(&m_MktMutex);
    if(_mktData.ticker == m_strIndependentTicker)
    {
        if(m_nIndependentMktUpdatePos > m_nDependentMktUpdatePos)
        {
            m_pIndependentMktData[m_nIndependentMktUpdatePos-1] = _mktData.Last * m_nIndependentMultiple;
        }
        else
        {
            m_pIndependentMktData[m_nIndependentMktUpdatePos] = _mktData.Last * m_nIndependentMultiple;
            ++m_nIndependentMktUpdatePos;
        }
    }

    if(_mktData.ticker == m_strDependentTicker)
    {
        if(m_nDependentMktUpdatePos > m_nIndependentMktUpdatePos)
        {
            m_pDependentMktData[m_nDependentMktUpdatePos-1] = _mktData.Last * m_nDependentMultiple;
        }
        else
        {
            m_pDependentMktData[m_nDependentMktUpdatePos] = _mktData.Last * m_nDependentMultiple;
            ++m_nDependentMktUpdatePos;
        }
    }
}   // updateMktData

bool CRegress::CalcRegressionParam()
{
    if(m_nIndependentMktUpdatePos < MKTDATAUSEDLENGTH || m_nDependentMktUpdatePos < MKTDATAUSEDLENGTH)
        return false;
    QMutexLocker locker(&m_MktMutex);

    // 从自变量和因变量的行情数组中分别读取MKTDATAUSEDLENGTH个最新行情
    double arrIndependentMktData[MKTDATAUSEDLENGTH];
    double arrDependentMktData[MKTDATAUSEDLENGTH];
    int nPos = std::min(m_nIndependentMktUpdatePos,m_nDependentMktUpdatePos);
    for(int i = 1;i <= MKTDATAUSEDLENGTH;++i)
    {
        arrIndependentMktData[i-1] = m_pIndependentMktData[nPos-i];
        arrDependentMktData[i-1] = m_pDependentMktData[nPos-i];
    }
    locker.unlock();

    // 采用最小二乘法计算回归参数
    // 回归方程 : y = a + bx + e
    // a = (sum(x_i*y_i) - n*mean(x)*mean(y))/(sum(x_i^2) - n*mean(x)*mean(x))
    // b = mean(y) - a*mean(x)
    // 相关系数rho = (n*sum(x_i*y_i) - sum(x_i)*sum(y_i))/sqrt((n*sum(y_i^2)-sum(y_i)*sum(y_i)) * (n*sum(x_i^2)-sum(x_i)*sum(x_i)))
    double fSumX = 0.0;
    double fSumY = 0.0;
    double fSumXY = 0.0;
    double fSumXX = 0.0;
    double fSumYY = 0.0;
    double fMeanX = 0.0;
    double fMeanY = 0.0;
    for(int i = 0;i < MKTDATAUSEDLENGTH;++i)
    {
        fSumX += arrIndependentMktData[i];
        fSumY += arrDependentMktData[i];
        fSumXX += arrIndependentMktData[i] * arrIndependentMktData[i];
        fSumYY += arrDependentMktData[i] * arrDependentMktData[i];
        fSumXY += arrIndependentMktData[i] * arrDependentMktData[i];
    }
    fMeanX = fSumX / MKTDATAUSEDLENGTH;
    fMeanY = fSumY / MKTDATAUSEDLENGTH;

    // 斜率
    double fSlopeUp = fSumXY - MKTDATAUSEDLENGTH * fMeanX * fMeanY;
    double fSlopeDown = fSumXX - MKTDATAUSEDLENGTH * fMeanX * fMeanX;
    double fSlope = fSlopeUp / fSlopeDown;
    // 截距项
    double fIntercept = fMeanY - fSlope * fMeanX;
    // 相关系数
    double fRhoUp = MKTDATAUSEDLENGTH * fSumXY - fSumX * fSumY;
    double fRhoDown = std::sqrt((MKTDATAUSEDLENGTH * fSumYY - fSumY * fSumY) - (MKTDATAUSEDLENGTH * fSumXX - fSumX * fSumX));
    double fRho = fRhoUp / fRhoDown;
    // 残差序列
    double fSumError = 0.0;     // 残差和
    double fMeanError = 0.0;    // 残差均值
    double arrError[MKTDATAUSEDLENGTH];     // 残差数组
    for(int i = 0;i < MKTDATAUSEDLENGTH;++i)
    {
        arrError[i] = arrDependentMktData[i] - fIntercept - arrIndependentMktData[i] * fSlope;
        fSumError += arrError[i];
    }
    fMeanError = fSumError / MKTDATAUSEDLENGTH;

    double fErrorSumSquare = 0.0;   // 残差与均值差的平方和
    double fErrorSumCube = 0.0;     // 残差与均值差的立方和
    double fErrorSum4Power = 0.0;   // 残差与均值差的4次方和
    for(int i = 0;i < MKTDATAUSEDLENGTH;++i)
    {
        doule fErrorDeviation = arrError[i] - fMeanError;
        fErrorSumSquare += fErrorDeviation * fErrorDeviation;
        fErrorSumCube += fErrorDeviation * fErrorDeviation * fErrorDeviation;
        fErrorSum4Power += fErrorDeviation * fErrorDeviation * fErrorDeviation * fErrorDeviation;
    }
    // 残差的标准差
    double fErrStd = std::sqrt(fErrorSumSquare/(MKTDATAUSEDLENGTH-1));
    // 残差的偏度skewness = 1/n * sum(e_i-mean(e))^3 / (1/(n-1) * sum(e_i-mean(e))^2)^(3/2)
    double fErrSkewnessUp = fErrorSumCube / MKTDATAUSEDLENGTH;
    double fErrSkewnessDown = std::sqrt((fErrorSumSquare/(MKTDATAUSEDLENGTH-1) * fErrorSumSquare/(MKTDATAUSEDLENGTH-1) * fErrorSumSquare/(MKTDATAUSEDLENGTH-1)));
    double fErrSkewness = fErrSkewnessUp / fErrSkewnessDown;
    // 残差的峰度kurtosis = 1/n * sum(e_i-mean(e))^4 / (1/n * sum(e_i - mean(e))^2)^2
    double fErrKurtosisUp = fErrorSum4Power / MKTDATAUSEDLENGTH;
    double fErrKurtosisDown = (fErrorSumSquare / MKTDATAUSEDLENGTH)*(fErrorSumSquare / MKTDATAUSEDLENGTH);
    double fErrKurtosis = fErrKurtosisUp / fErrKurtosisDown;

    // 计算残差偏度对开仓阈值的影响
    double fSkewnessEffect = 0.0;
    if(fErrSkewness > 0)
        // 正偏度，提高上限，降低下限
        fSkewnessEffect = std::min(fErrSkewness,0.01);
    else
        // 负偏态，降低上限，提高下限
        fSkewnessEffect = std::max(fErrSkewness,-0.01);
    // 计算残差峰度对开仓阈值的影响
    double fKurtosisEffect = 0.0;
    if(fErrKurtosis > 3.3)
        // 尖峰、肥尾，异常值偏多，提高上下限
        fKurtosisEffect = std::max(0.01,1.0/fErrKurtosis/30.0);
    else
    {
        if(fErrKurtosis < 2.5)
            // 平缓廋尾巴，减少上下限
            fKurtosisEffect = 0.0 - std::min(0.01,std::min(fErrKurtosis,1.0/fErrKurtosis)/40.0);
        else
            fKurtosisEffect = 0.0;
    }
    // 计算开仓残差值上下限
    double fOpenUpperLimit = (1.0+1.0/fRho+fSkewnessEffect+fKurtosisEffect)*fErrStd;
    double fOpenLowerLimit = -(1.0+1.0/fRho-fSkewnessEffect+fKurtosisEffect)*fErrStd;
    // 计算止盈止损时对应的残差变动值
    double fErrorChgForExitTrade = std::min(1.0+fKurtosisEffect+std::abs(fSkewnessEffect),1.0/fRho)*fErrStd;

    // 更新参数
    m_tRegressParam.strIndependentTicker = m_strIndependentTicker;
    m_tRegressParam.strDependentTicker = m_strDependentTicker;
    m_tRegressParam.fIntercept = fIntercept;
    m_tRegressParam.fSlope = fSlope;
    m_tRegressParam.fErrorStd = fErrStd;
    m_tRegressParam.fRho = fRho;
    m_tRegressParam.fSkewness = fErrSkewness;
    m_tRegressParam.fKurtosis = fErrKurtosis;
    m_tRegressParam.fOpenUpperLimit = fOpenUpperLimit;
    m_tRegressParam.fOpenLowerLimit = fOpenLowerLimit;
    m_tRegressParam.fErrorChgForExitTrade = fErrorChgForExitTrade;

    // 发送计算结果
    emit CalculationDone(m_tRegressParam);

}   // CalcRegressionParam

}
