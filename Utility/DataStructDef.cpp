#include "DataStructDef.h"

namespace HESS
{

// 拷贝衍生品深度行情数据
void CopyDerivativeDepthMktDataField(DerivDepthMktDataField& dest,const DerivDepthMktDataField& source)
{
    dest.TradingDay = source.TradingDay;
    dest.UpdateTime = source.UpdateTime;
    dest.InstrumentID = source.InstrumentID;
    dest.ExchangeID = source.ExchangeID;
    dest.ExchangeInstrID = source.ExchangeInstrID;
    dest.Last = source.Last;
    dest.PreSettlement = source.PreSettlement;
    dest.PreClose = source.PreClose;
    dest.PreOpenInsterest = source.PreOpenInsterest;
    dest.Open = source.Open;
    dest.High = source.High;
    dest.Low = source.Low;
    dest.Volume = source.Volume;
    dest.Turnover = source.Turnover;
    dest.OpenInterest = source.OpenInterest;
    dest.Close = source.Close;
    dest.Settlement = source.Settlement;
    dest.UpperLimit = source.UpperLimit;
    dest.LowerLimit = source.LowerLimit;
    dest.PreDelta = source.PreDelta;
    dest.CurrDelta = source.CurrDelta;
    dest.AskPrice[0] = source.AskPrice[0];
    dest.AskPrice[1] = source.AskPrice[1];
    dest.AskPrice[2] = source.AskPrice[2];
    dest.AskPrice[3] = source.AskPrice[3];
    dest.AskPrice[4] = source.AskPrice[4];
    dest.AskVol[0] = source.AskVol[0];
    dest.AskVol[1] = source.AskVol[1];
    dest.AskVol[2] = source.AskVol[2];
    dest.AskVol[3] = source.AskVol[3];
    dest.AskVol[4] = source.AskVol[4];
    dest.BidPrice[0] = source.BidPrice[0];
    dest.BidPrice[1] = source.BidPrice[1];
    dest.BidPrice[2] = source.BidPrice[2];
    dest.BidPrice[3] = source.BidPrice[3];
    dest.BidPrice[4] = source.BidPrice[4];
    dest.BidVol[0] = source.BidVol[0];
    dest.BidVol[1] = source.BidVol[1];
    dest.BidVol[2] = source.BidVol[2];
    dest.BidVol[3] = source.BidVol[3];
    dest.BidVol[4] = source.BidVol[4];
    dest.AvgPrice = source.AvgPrice;
}

void CopyDerivativeDepthMktDataField(DerivDepthMktDataField &dest, const CThostFtdcDepthMarketDataField &source)
{
    dest.TradingDay = QString(source.TradingDay);
    dest.UpdateTime = QString(source.UpdateTime);
    dest.InstrumentID = QString(source.InstrumentID);
    dest.ExchangeID = QString(source.ExchangeID);
    dest.ExchangeInstrID = QString(source.ExchangeInstID);
    dest.Last = source.LastPrice;
    dest.PreSettlement = source.PreSettlementPrice;
    dest.PreClose = source.PreClosePrice;
    dest.PreOpenInsterest = source.PreOpenInterest;
    dest.Open = source.OpenPrice;
    dest.High = source.HighestPrice;
    dest.Low = source.LowestPrice;
    dest.Volume = source.Volume;
    dest.Turnover = source.Turnover;
    dest.OpenInterest = source.OpenInterest;
    dest.Close = source.ClosePrice;
    dest.Settlement = source.SettlementPrice;
    dest.UpperLimit = source.UpperLimitPrice;
    dest.LowerLimit = source.LowerLimitPrice;
    dest.PreDelta = source.PreDelta;
    dest.CurrDelta = source.CurrDelta;
    dest.AskPrice[0] = source.AskPrice1;
    dest.AskPrice[1] = source.AskPrice2;
    dest.AskPrice[2] = source.AskPrice3;
    dest.AskPrice[3] = source.AskPrice4;
    dest.AskPrice[4] = source.AskPrice5;
    dest.AskVol[0] = source.AskVolume1;
    dest.AskVol[1] = source.AskVolume2;
    dest.AskVol[2] = source.AskVolume3;
    dest.AskVol[3] = source.AskVolume4;
    dest.AskVol[4] = source.AskVolume5;
    dest.BidPrice[0] = source.BidPrice1;
    dest.BidPrice[1] = source.BidPrice2;
    dest.BidPrice[2] = source.BidPrice3;
    dest.BidPrice[3] = source.BidPrice4;
    dest.BidPrice[4] = source.BidPrice5;
    dest.BidVol[0] = source.BidVolume1;
    dest.BidVol[1] = source.BidVolume2;
    dest.BidVol[2] = source.BidVolume3;
    dest.BidVol[3] = source.BidVolume4;
    dest.BidVol[4] = source.BidVolume5;
    dest.AvgPrice = source.AveragePrice;
}

void TDerivativeMktDataField::update(const CThostFtdcDepthMarketDataField &_mktData)
{
    // lock the mutex locker

    // update the newest mkt data
    Last = _mktData.LastPrice;
    AskPrice[0] = _mktData.AskPrice1;
    AskVol[0] = _mktData.AskVolume1;
    BidPrice[0] = _mktData.BidPrice1;
    BidVol[0] = _mktData.BidVolume1;
}

}
