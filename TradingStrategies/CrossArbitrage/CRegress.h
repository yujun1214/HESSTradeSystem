#ifndef CREGRESS_H
#define CREGRESS_H

/*����ع������������㲥������*/
#include <QObject>
#include "Utility/DataStructDef.h"
#include <QMutex>

namespace HESS
{

#define MKTDATAARRAYLENGTH 40000    // �������鳤��
#define MKTDATAUSEDLENGTH 512       // ����ع���������õ��������ݳ���

class CRegress:public QObject
{
    Q_OBJECT
public:
    CRegress(QString _strIndependentTicker, QString _strDependentTicker);
    ~CRegress();

//    void addRegressPair(QString strIndependentTicker,QString strDependentTicker);

private:
    // �ع��������ṹ
    TCointegrationParam m_tRegressParam;

    // �Ա�������
    QString m_strIndependentTicker;
    // ���������
    QString m_strDependentTicker;

    // �Ա����ĳ���
    int m_nIndependentMultiple;
    // ������ĳ���
    int m_nDependentMultiple;

    // �Ա����������飬�ڶ��Ϸ��䣬Ĭ�ϳ���Ϊ40000��
    double* m_pIndependentMktData;
    // ������������ݣ��ڶ��Ϸ��䣬Ĭ�ϳ���Ϊ40000��
    double* m_pDependentMktData;
    // �������ݵĻ�����
    QMutex m_MktMutex;

    // �Ա�����������ĸ���λ��
    int m_nIndependentMktUpdatePos;
    // �������������ĸ���λ��
    int m_nDependentMktUpdatePos;

    // ����ع���������õ��������ݳ���
//    int m_nMktDataUsedLength;

private:
    // ����ع����
    bool CalcRegressionParam();

public slots:
    // �������飬���ж������Ƿ�һ��
    void updateMktData(TDerivativeMktDataField _mktData);

signals:
    // ���ͼ�����
    void CalculationDone(TCointegrationParam _param);
};

}
#endif // CREGRESS_H
