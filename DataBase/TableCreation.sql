use Portfolio;
/* 组合交易清单表(TradeList) */
Create Table TradeList(
	tmDate			varchar(8)		not null,	/* 日期 */
	Portfolio		varchar(50)		not null,	/* 组合名称 */
	Strategy		varchar(50)		not null,	/* 策略名称 */
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	SecuName		varchar(20)		not null,	/* 证券简称 */
	SecuType		varchar(20)		not null,	/* 证券类别（股票、股指期货、ETF、债券） */
	Exchange		varchar(20)		not null,	/* 交易所 */
	Industry		varchar(50)		not null,	/* 行业 */
	TradeType		varchar(10)		not null,	/* 交易类型（股票：买入、卖出、分红、送股；ETF:买入、卖出；期指：买入开仓、卖出平仓、卖出开仓、买入平仓；信用交易：融券卖出、买券还券、融资买入、卖券还款） */
	IsCreditTrade	int				not null,	/* 是否为信用交易（TradeType为信用交易类型时 = 1；否则 = 0） */
	TradeVol		numeric(20,4)	not null,	/* 交易量（股票：股；期货：手） */
	TradePrice		numeric(20,4)	not null,	/* 交易价格 */
	PriceType		varchar(50)		not null	/* 价格类型(PrevClose=前收盘价、PrevSettlement=前结算价、Open=开盘价、Close=收盘价、Settlement=结算价、VWAP=均价、Real=实际成交均价 */
);

/* 组合基本信息 */
Create Table PortfolioBasicInfo(
	PortfolioName		varchar(50)		not null,	/* 组合名称 */
	PortfolioType		varchar(20)		not null,		/* 组合类型（实盘组合，模拟组合）*/
	Description				varchar(200)	not null,	/* 组合描述 */
	primary key (PortfolioName)
);

/* 组合交易表(PortfolioTrade)，组合实际的成交回报 */
create table PortfolioTrade(
	tmDate			varchar(8)		not null,	/*日期*/
	Portfolio		varchar(50)		not null,	/* 组合名称 */
	Strategy		varchar(50) 	not null,	/* 策略名称 */
	SecuCode		varchar(10) 	not null,	/* 证券代码 */
	SecuName		varchar(20) 	not null,	/* 证券简称 */
	SecuType		varchar(20) 	not null,	/* 证券类别（股票、股指期货、ETF、债券） */
	Exchange		varchar(20)		not null,	/* 交易所 */
	Industry		varchar(50) 	not null,	/* 行业（中信二级行业） */
	TradeType		varchar(10) 	not null,	/* 交易类型（股票：买入、卖出、分红、送股；ETF:买入、卖出；期指：买入开仓、卖出平仓、卖出开仓、买入平仓；信用交易：融券卖出、买券还券、融资买入、卖券还款） */
	IsCreditTrade	int 			not null,	/* 是否为信用交易,0=否；1=是(TradeType为信用交易类型时，=1；否则=0) */
	TradeVol		numeric(20,4)	not null,	/* 交易量（股票：股；期指：手） */
	TradePrice		numeric(20,4) 	not null,	/* 交易价格 */
	PriceType		varchar(50) 	not null,	/* 价格类型（PrevClose=前收盘价、PrevSettlement=前结算价、Open=开盘价、Close=收盘价、Settlement=结算价、VWAP=均价、Real=实际成交均价） */
	TradeValue		numeric(20,4) 	not null,	/* 交易金额 */
	TradeCost		numeric(20,4) 	not null,	/* 交易费用 */
	MarketRatio		numeric(18,6) 	not null,	/* 占市场交易量比例 */
	PrevHoldingID	int 			null		/* 对应前一天的持仓ID */
);

/* 组合持仓表(PortfolioHoldings) */
create table PortfolioHoldings(
	tmDate	varchar(8) not null,	/* 日期 */
	Portfolio	varchar(50) not null,	/* 组合名称 */
	Strategy	varchar(50) not null,	/* 策略名称 */
	SecuCode	varchar(10) not null,	/* 证券代码(保证金=999999,现金=888888) */
	SecuName	varchar(20) not null,	/* 证券简称 (保证金=Margin,现金=Cash)*/
	SecuType	varchar(20) not null,	/* 证券类别（股票、ETF、股指期货、保证金(用于计算衍生品的盈亏)、现金） */
	Exchange	varchar(20)	not null,	/* 交易所 */
	Industry	varchar(50) not null,	/* 行业（中信二级行业） */
	HoldingStatus	varchar(10) not null,	/* 多空状态（多仓、空仓） */
	IsCreditTrade	int not null,					/* 是否为信用交易，0=否；1=是 */
	Holding		numeric(20,4) not null,	/* 持仓量（股票：股；期指：手；保证金：元） */
	Price			numeric(18,4)	not null,	/* 估值价格（股票：收盘价；期指：结算价；保证金：1元） */
	MTMValue	numeric(20,4) not null,	/* 估值价值（股票：收盘价计算；期指：结算价计算） */
	Cost			numeric(20,4) not null,	/* 持仓成本 */
	AccumulativeRight	numeric(20,4) not null,	/* 累计权益，用于计算融券卖空的累计分红 */
	ID				int not null,	/* 持仓ID号，同一天、同一组合、同一策略的持仓ID不重复 */
	primary key (tmDate,Portfolio,Strategy,SecuCode,SecuType,HoldingStatus,IsCreditTrade)
);

/* 组合现金流表(PortfolioCashflow)，记录实际交易后，组合流入流出的现金额（作为申购赎回处理） */
Create Table PortfolioCashflow(
	tmDate		varchar(8) not null,	/* 日期 */
	Portfolio	varchar(50) not null,	/* 组合名称 */
	Strategy	varchar(50) not null,	/* 策略名称 */
	IsPortfolio	int not null,			/* 是否是组合现金流数据,0=否；1=是 */
	CashInflow	numeric(20,4) not null,	/* 现金流入金额 */
	CashOutflow	numeric(20,4) not null,	/* 现金流出金额 */
	primary key (tmDate,Portfolio,Strategy,IsPortfolio)
);

/*组合估值表(PortfolioValuation)*/
Create Table PortfolioValuation(
	tmDate		varchar(8) not null,	/* 日期 */
	Portfolio	varchar(50) not null,	/* 组合名称 */
	Strategy	varchar(50) not null,	/* 策略名称 */
	IsPortfolio	int not null,				/* 是否是组合层面估值数据,0=否；1=是 */
	TotalUnits	numeric(20,4) not null,	/* 组合份额 */
	Cost		numeric(20,4) not null,	/* 组合成本 */
	NAV			numeric(20,4) not null,	/* 组合净值 */
	UnitNAV		numeric(18,4) not null,	/* 组合单位净值 */
	TWR			numeric(18,4) not null,	/* 组合TWR */
	ProfitAndLoss	numeric(20,4)	not null,	/* 组合当日P&L */
	AccuProfitAndLoss	numeric(20,4)	not null,	/* 组合累计P&L */
	primary key (tmDate,Portfolio,Strategy,IsPortfolio)
);

/* 组合费率表(PortfolioFee) */
Create Table PortfolioFee(
	tmDate				varchar(8) not null,	/* 日期 */
	Portfolio			varchar(50) not null,	/* 组合名称 */
	StockBuyFeeRatio	numeric(18,6) not null,	/* 股票买入费率 */
	StockSellFeeRatio	numeric(18,6) not null,	/* 股票卖出费率 */
	ETFBuyFeeRatio		numeric(18,6) not null,	/* ETF买入费率 */
	ETFSellFeeRatio		numeric(18,6) not null,	/* ETF卖出费率 */
	IndexFutureTradeFeeRatio			numeric(18,6) not null,	/* 股指期货交易费率 */
	primary key (tmDate,Portfolio)
);

/* 信用交易费率(CreditTradeFee) */
Create Table CreditTradeFee(
	tmDate				varchar(8) not null,	/* 日期 */
	Portfolio			varchar(50) not null,	/* 组合名称 */
	BorrowingFeeRatio	numeric(18,6) not null,	/* 融券费率 */
	MarginFeeRatio		numeric(18,6) not null,	/* 融资费率 */
	primary key (tmDate,Portfolio)
);

/* 策略配置表(StrategyAlloc) */
Create Table StrategyAlloc(
	Strategy	varchar(50)	not null,	/* 策略名称 */
	Portfolio	varchar(50)	not null,	/* 组合名称 */
	tmBegDate	varchar(8)	not null,	/* 配置开始日期 */
	tmEndDate	varchar(8)	not null	/* 配置结束日期 */
);

---------------------------策略回测数据库---------------------
use StrategyData;
/* 策略基本信息 */
Create Table StrategyBasicInfo(
	StrategyCode	varchar(50)		not null,	/* 策略代码 */
	StrategyName	varchar(50)		not null,	/* 策略名称 */
	primary key (StrategyCode)
);

/* 策略持仓权重表 */
Create Table StrategyHoldingWeight(
	tmDate 			varchar(8)		not null,	/* 日期 */
	StrategyCode	varchar(50)		not null,	/* 策略代码 */
	SecuCode		varchar(10)		not null,	/* 个股代码 */
	Weight 			numeric(18,6)	not null,	/* 个股权重 */
);

/* 策略收益表 */
Create Table StrategyReturn(
	tmDate 			varchar(8)		not null,	/* 日期 */
	StrategyCode 	varchar(50)		not null,	/* 策略代码 */
	TWR 			numeric(18,6)	not null,	/* 策略TWR */
	primary key (tmDate,StrategyCode)
);


---------------------------期权交易相关表--------------------
use OptionArbitrade;
-- 期权套利交易表(箱型价差套利)
Create Table OptArbTradeXT(
	tmDate				varchar(8)		not null,	/* 交易日期(yyyyMMdd) */
	time				varchar(8)		not null,	/* 交易时间(hh:mm:ss) */
	Portfolio			varchar(50)		not null,	/* 组合名称 */
	StrategyType		varchar(10)		not null,	/* 策略类型(buy=买入箱型价差套利策略,sell=卖出箱型价差套利策略) */
	StrategyTradeType	varchar(10)		not null,	/* 策略的交易类型(open=策略开仓交易，close=策略平仓交易) */
	Call1Code			varchar(20)		not null,	/* 行权价为K1的call代码 */
	Call1Name			varchar(50)		not null,	/* 行权价为K1的call名称 */
	Call1TradeDirection	varchar(10)		not null,	/* 行权价为K1的call的买卖方向(buy,sell,exercise) */
	Call1TradeVol		numeric(20,4)	not null,	/* 行权价为K1的call的交易量 */
	Call1TradePrice		numeric(20,4)	not null,	/* 行权价为K1的call的交易价格 */
	Call1TradeValue		numeric(20,4)	not null,	/* 行权价为K1的call的交易金额 */
	Call2Code			varchar(20)		not null,	/* 行权价为K2的call代码 */
	Call2Name			varchar(50)		not null,	/* 行权价为K2的call名称 */
	Call2TradeDirection	varchar(10)		not null,	/* 行权价为K2的call的买卖方向(buy,sell,exercise) */
	Call2TradeVol		numeric(20,4)	not null,	/* 行权价为K2的call的交易量 */
	Call2TradePrice		numeric(20,4)	not null,	/* 行权价为K2的call的交易价格 */
	Call2TradeValue		numeric(20,4)	not null,	/* 行权价为K2的call的交易金额 */
	Put1Code			varchar(20)		not null,	/* 行权价为K1的put代码 */
	Put1Name			varchar(50)		not null,	/* 行权价为K1的put名称 */
	Put1TradeDirection	varchar(10)		not null,	/* 行权价为K1的put的买卖方向(buy,sell,exercise) */
	Putl1TradeVol		numeric(20,4)	not null,	/* 行权价为K1的put的交易量 */
	Put1TradePrice		numeric(20,4)	not null,	/* 行权价为K1的put的交易价格 */
	Put1TradeValue		numeric(20,4)	not null,	/* 行权价为K1的put的交易金额 */
	Put2Code			varchar(20)		not null,	/* 行权价为K2的put代码 */
	Put2Name			varchar(50)		not null,	/* 行权价为K2的put名称 */
	Put2TradeDirection	varchar(10)		not null,	/* 行权价为K2的put的买卖方向(buy,sell,exercise) */
	Put2TradeVol		numeric(20,4)	not null,	/* 行权价为K2的put的交易量 */
	Put2TradePrice		numeric(20,4)	not null,	/* 行权价为K2的put的交易价格 */
	Put2TradeValue		numeric(20,4)	not null,	/* 行权价为K2的put的交易金额 */
	CashAmount			numeric(20,4)	not null,	/* 现金交易额 */
	CashTradeDirection	varchar(10)		not null,	/* 现金的交易方向(borrow=借入,lend=借出) */
	InterestRate		numeric(20,4)	not null,	/* 资金利率 */
	TradeCost			numeric(20,4)	not null,	/* 本次交易交易费用 */
	OptArbID			int				not null,	/* 套利交易ID(唯一标识 = yyyyMMdd+序号,如果是策略平仓交易,那么OptArbID与该笔策略开仓交易相同) */
	primary key (OptArbID,StrategyTradeType)
);

-- 期权套利交易表
Create Table OptArbTrade(
	tmDate			varchar(8)		not null,	/* 交易日期(yyyyMMdd) */
	time			varchar(8)		not null,	/* 交易时间(hh:mm:ss) */
	TradeID			varchar(20)				not null,	/* 交易号,同一组套利交易的交易号相同 */
	Portfolio		varchar(50)		not null,	/* 组合名称(账户的UserID) */
	StrategyName	varchar(50)		not null,	/* 套利策略名称(BuyBoxSpread,SellBoxSpread,ShortCallSpread,ShortPutSpread) */
	OpenClose		varchar(10)		not null,	/* 策略开平仓(开仓交易=open;平仓交易=close) */
	SecuCode		varchar(20)		not null,	/* 证券代码 */
--	SecuName		varchar(50)		not null,	/* 证券名称 */
	SecuType		varchar(10)		not null,	/* 证券类型(认购期权=call,认沽期权=put,股票=stock,交易所交易基金etf,现金=cash) */
	TradeDirection	varchar(10)		not null,	/* 交易方向(买入=buy,卖出=sell,期权行权=exec) */
	TradeVol		int				not null,	/* 成交数量(股票=股,期权=张,ETF=份) */
	TradePrice		numeric(20,4)	not null,	/* 成交价格 */
	TradeValue		numeric(20,4)	not null,	/* 成交金额 */
--	BorrowRate		numeric(20,4)	not null,	/* 借入资金利率(该笔套利交易在开仓时的BorrowRate参数值) */
--	LendRate		numeric(20,4)	not null,	/* 借出资金利率(该笔套利交易在开仓时的LendRate参数值) */
	ClosedVol		int				not null,	/* 已平仓数量 */
	SecuTypeSymbol	varchar(20)		not null,	/* 证券类型标识 */
	primary key (TradeID,Portfolio,SecuCode,OpenClose)
);

-- 期权套利交易参数表
Create Table OptArbTradeParam(
	tmDate		varchar(8)		not null,	/* 交易日期(yyyyMMdd) */
	ParamName	varchar(50)		not null,	/* 参数名称 */
	ParamValue	numeric(20,4)	not null,	/* 参数值 */
	primary key (tmDate,ParamName)
)

-- 期权套利持仓表(箱体价差套利)
Create Table OptArbHoldingsXT(
	tmDate				varchar(8)		not null,	/* 日期 */
	Portfolio			varchar(50)		not null,	/* 组合名称 */
	StrategyType		varchar(10)		not null,	/* 策略类型(buy=买入箱型价差套利策略,sell=卖出箱型价差套利策略) */
	Call1Code			varchar(20)		not null,	/* 行权价为K1的call的代码 */
	Call1Name			varchar(50)		not null,	/* 行权价为K1的call的名称 */
	Call1Holding		numeric(20,4)	not null,	/* 行权价为K1的call的持仓量 */
	Call1HoldingStatus	varchar(10)		not null,	/* 行权价为K1的call的多空状态(long,short) */
	Call1Cost			numeric(20,4)	not null,	/* 行权价为K1的call的单位持仓成本 */
	Call1ClosedPrice	numeric(20,4)	null,		/* 行权价为K1的call的平仓价格 */
	Call1IsExercised	int				not null,	/* 行权价为K1的call是否被行权(0=未行权,1=已行权) */
	Call1ExerciseProfit	numeric(20,4)	not null,	/* 行权价为K1的call的行权收益 */
	Call2Code			varchar(20)		not null,	/* 行权价为K2的call的代码 */
	Call2Name			varchar(50)		not null,	/* 行权价为K2的call的名称 */
	Call2Holding		numeric(20,4)	not null,	/* 行权价为K2的call的持仓量 */
	Call2HoldingStatus	varchar(10)		not null,	/* 行权价为K2的call的多空状态(long,short) */
	Call2Cost			numeric(20,4)	not null,	/* 行权价为K2的call的单位持仓成本 */
	Call2ClosedPrice	numeric(20,4)	null,		/* 行权价为K2的call的平仓价格 */
	Call2IsExercised	int				not null,	/* 行权价为K2的call是否被行权(0=未行权,1=已行权) */
	Cal2ExerciseProfit	numeric(20,4)	not null,	/* 行权价为K2的call的行权收益 */
	Put1Code			varchar(20)		not null,	/* 行权价为K1的put的代码 */
	Put1Name			varchar(50)		not null,	/* 行权价为K1的put的名称 */
	Put1Holding			numeric(20,4)	not null,	/* 行权价为K1的put的持仓量 */
	Put1HoldingStatus	varchar(10)		not null,	/* 行权价为K1的put的多空状态(long,short) */
	Put1Cost			numeric(20,4)	not null,	/* 行权价为K2的put的单位持仓成本 */
	Put1ClosedPrice		numeric(20,4)	null,		/* 行权价为K2的put的平仓价格 */
	Put1IsExercised		int				not null,	/* 行权价为K1的put是否被行权(0=未行权,1=已行权) */
	Put1ExerciseProfit	numeric(20,4)	not null,	/* 行权价为K1的put的行权收益 */
	Put2Code			varchar(20)		not null,	/* 行权价为K2的put的代码 */
	Put2Name			varchar(50)		not null,	/* 行权价为K2的put的名称 */
	Put2Holding			numeric(20,4)	not null,	/* 行权价为K2的put的持仓量 */
	Put2HoldingStatus	varchar(10)		not null,	/* 行权价为K2的put的多空状态(long,short) */
	Put2Cost			numeric(20,4)	not null,	/* 行权价为K2的put的单位持仓成本 */
	Put2ClosedPrice		numeric(20,4)	null,		/* 行权价为K2的put的平仓价格 */
	Put2IsExercised		int				not null,	/* 行权价为K2的put是否被行权(0=未行权,1=已行权) */
	Put2ExerciseProfit	numeric(20,4)	not null,	/* 行权价为K2的put的行权收益 */
	CashAmount			numeric(20,4)	not null,	/* 现金额度(按资金利率每天计息) */
	CashHoldingStatus	varchar(10)		not null,	/* 现金的持仓状态(borrow=借入,lend=借出) */
	InterestRate		numeric(20,4)	not null,	/* 资金利率 */
	ArbRet				numeric(20,4)	not null,	/* t套利收益(该字段在策略平仓或行权后计算) */
	isClosed			int				not null,	/* 该套利交易是否平仓(未平仓=0,已平仓=1) */
	OptArbID			int				not null,	/* 持仓ID号(唯一标识=yyyyMMdd+序号,与箱型价差套利交易表中的OptArbID对应) */
	primary key (OptArbID)
);

use BasicData;
-- 业绩预告信息表(PerformanceAnnounceInfo)
Create Table PerformanceAnnounceInfo(
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	SecuName		varchar(20)		not null,	/* 证券简称 */
	Exchange		varchar(20)		not null,	/* 交易所 */
	tmPubDate		varchar(8)		not null,	/* 公告日期 */
	ReportDate		varchar(8)		not null,	/* 业绩报告期 */	
	AnnounceType	varchar(20)		not null,	/* 预告类型：首亏、续亏、扭亏、续盈、预增、预减、略增、略减、不确定、其他 */
	AnnounceTyeCode	int					not null,	/* 预告类型代码：0=其他;1=不确定;2=续亏;3=首亏;4=略减;5=预减;6=扭亏;7=续盈;8=略增;9=预增 */
	NetProfitGrowthFloor	numeric(20,4)	null,	/* 预告净利润变动幅度下限 */
	NetProfitGrowthCeiling	numeric(20,4)	null,	/* 预告净利润变动幅度上限 */
	EarningFloor		numeric(20,4)	null,	/* 预计收入下限 */
	EarningCeiling		numeric(20,4)	null,	/* 预计收入上限 */
	NetProfitFloor		numeric(20,4)	null,	/* 预告净利润下限 */
	NetProfitCeiling	numeric(20,4)	null,	/* 预告净利润上限 */
	NetProfit			numeric(20,4)	null,	/* 预告净利润变动幅度(只公布一个变动幅度值【上限、下限或数值】的加工公布值) */
	EPSFloor			numeric(20,4)	null,	/* 预计EPS下限 */
	EPSCeiling			numeric(20,4)	null,	/* 预计EPS上限 */
	primary key (SecuCode,tmPubDate,ReportDate)
);

-- 证券基本信息表(SecuBasicInfo)
Create Table SecuBasicInfo(
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	SecuName		varchar(20)		not null,	/* 证券简称 */
	SecuType		varchar(20)		not null,	/* 证券类型（股票、ETF、债券、指数、基金） */
	Exchange		varchar(20)		not null,	/* 交易所（上交所、深交所、场外） */
	ListedDate		varchar(8)		not null,	/* 上市交易日期 */
	DelistedDate	varchar(8)		not null,	/* 摘牌日期 */
	Currency		varchar(10)		not null,	/* 交易货币 */
	primary key (SecuCode)
);
	
-- 证券停牌信息(SecuSuspendInfo)
Create Table SecuSuspendInfo(
	SecuCode	varchar(10)		not null,	/* 证券代码 */
	-- Exchange	varchar(20)		not null,	/* 交易所 */
	SuspendDate	varchar(8)		not null,	/* 停牌日期 */
	SuspendType	varchar(20)		not null,	/* 停牌类型(今起停牌、停牌一天......) */
	Description	varchar(400)	null,		/* 停牌原因描述 */
	primary key (SecuCode,SuspendDate)
);

-- 沪深交易所证券日行情数据(SecuQuotationData)
Create Table SecuQuotationData(
	tmDate			varchar(8)		not null,	/* 日期 */
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	Exchange		varchar(20)		not null,	/* 交易所 */
	PrevClose		decimal(20,4)	not null,	/* 前收盘价 */
	Open			decimal(20,4)	not null,	/* 开盘价 */
	High			decimal(20,4)	not null,	/* 最高价 */
	Low				decimal(20,4)	not null,	/* 最低价 */
	Close			decimal(20,4)	not null,	/* 收盘价 */
	Volume			decimal(20,4)	not null,	/* 成交量（股、张） */
	Amount			decimal(20,4)	not null,	/* 成交额（元）*/
	primary	key (tmDate,SecuCode)
);

-- 沪深交易所日复权行情数据(SecuRestorationQuotationData)
Create Table SecuRestorationQuotationData(
	tmDate			varchar(8)		not null,	/* 日期 */
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	Exchange		varchar(20)		not null,	/* 交易所 */
	PrevClose		decimal(20,4)	not null,	/* 复权前收盘价 */
	Open			decimal(20,4)	not null,	/* 复权开盘价 */
	High			decimal(20,4)	not null,	/* 复权最高价 */
	Low				decimal(20,4)	not null,	/* 复权最低价 */
	Close			decimal(20,4)	not null,	/* 复权收盘价 */
	Volume			decimal(20,4)	not null,	/* 成交量（股、张） */
	Amount			decimal(20,4)	not null,	/* 成交额（元） */
	Factor			float			not null,	/* 复权因子，初始值为1；当日复权因子=前一日收盘价/当日前收盘价*前一交易日复权因子 */
	primary key (tmDate,SecuCode)
);

-- 沪深交易所1分钟行情数据(Secu1MQuotationData)
Create Table Secu1MQuotationData(
	tmDate 			varchar(8) 		not null,	/* 日期(yyyymmdd) */
	tmTime 			varchar(6)		not null,	/* 时间(hhmmss) */
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	PrevClose 		numeric(20,4)	not null,	/* 前收盘价 */
	Open 			numeric(20,4)	not null,	/* 开盘价 */
	High 			numeric(20,4)	not null,	/* 最高价 */
	Low 			numeric(20,4)	not null,	/* 最低价 */
	Close 			numeric(20,4)	not null,	/* 收盘价 */
	Volume 			numeric(20,4)	not null,	/* 成交量(股) */
	Amount 			numeric(20,4)	not null,	/* 成交额(元) */
	primary key (tmDate,tmTime,SecuCode)
);

-- 沪深交易所1分钟复权行情数据(Secu1MRstQuotationData)
Create Table Secu1MRstQuotationData(
	tmDate 			varchar(8) 		not null,	/* 日期(yyyymmdd) */
	tmTime 			varchar(6)		not null,	/* 时间(hhmmss) */
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	PrevClose 		numeric(20,4)	not null,	/* 前收盘价 */
	Open 			numeric(20,4)	not null,	/* 开盘价 */
	High 			numeric(20,4)	not null,	/* 最高价 */
	Low 			numeric(20,4)	not null,	/* 最低价 */
	Close 			numeric(20,4)	not null,	/* 收盘价 */
	Volume 			numeric(20,4)	not null,	/* 成交量(股) */
	Amount 			numeric(20,4)	not null,	/* 成交额(元) */
	Factor 			numeric(20,4)	not null,	/* 复权因子,复权价/复权因子=原始价格 */
	primary key (tmDate,tmTime,SecuCode)
);

-- 沪深交易所交易日（HSTradingDay）
Create Table HSTradingDay(
	tmTradingDay	varchar(8)		not null	/* 沪深交易所交易日 */
);

-- 配股数据（AllotmentData）
Create Table AllotmentData(
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	SecuName		varchar(20)		not null,	/* 证券简称 */
	-- Exchange		varchar(20)		not null,	/* 交易所 */
	ReportDate		varchar(8)		not null,	/* 公告日期 */
	AllotmentYear	int		not null,	/* 配股年度 */
	AllotmentPrice	decimal(18,4)	not null,	/* 配股价格 */
	-- Currency		varchar(10)		not null,	/* 货币 */
	RegistrationDate	varchar(8)	not null,	/* 股权登记日 */
	ExRightDate		varchar(8)		not null,	/* 除权日 */
	AllotmentListedDate	varchar(8)	not null,	/* 配股上市日 */
	AllotmentRatio	decimal(18,4)	not null,	/* 配股比例（每股配股比例） */
	primary key (SecuCode,RegistrationDate)
);

-- 分红数据（DividendData）
Create Table DividendData(
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	SecuName		varchar(20)		not null,	/* 证券简称 */
	-- Exchange		varchar(20)		not null,	/* 交易所 */
	ReportDate		varchar(8)		not null,	/* 公告日期 */
	DividendYear	varchar(8)		not null,	/* 分红年度 */
	RegistrationDate	varchar(8)	not null,	/* 股权登记日 */
	ExRightDate			varchar(8)	not null,	/* 除息日 */
	ShareDividendRatio	decimal(18,4)	not null,	/* 每股送股比例 */
	ShareTransferRatio	decimal(18,4)	not null,	/* 每股转增比例 */
	CashDividendRatio_PreTax	decimal(18,4)	not null,	/* 每股派息比例(税前) */
	CashDividendRatio_AfterTax	decimal(18,4)	not null,	/* 每股派息比例(税后) */
	BenchmarkShares	decimal(20,4)	not null,	/* 基准股本 */
	primary key (SecuCode,RegistrationDate,DividendYear)
);

-- 证券股本结构数据
Create Table ShareStructureData(
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	SecuName		varchar(20)		not null,	/* 证券简称 */
	tmPubDate		varchar(8)		not null,	/* 公告日期 */
	tmEndDate		varchar(8)		not null,	/* 股本截止日期 */
	TotalShares	numeric(20,4)	not null,	/* 总股本(股) */
	AShares			numeric(20,4)	not null,	/* A股总股本(股) */
	AFloats			numeric(20,4)	not null,	/* 流通A股(股) */
	RestrictedAShares	numeric(20,4)	not null,	/* 有限售条件的流通A股(股) */
	NonRestrictedShares	numeric(20,4)	not null,	/* 无限售条件的流通A股(股) */
	NonListedShares		numeric(20,4)	not null,	/* 未流通A股(股) */
	primary key (SecuCode,tmPubDate,tmEndDate)
);

-- 证券行业分类(SecuIndustryClassify)
Create Table SecuIndustryClassify(
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	Exchange		varchar(20)		not null,	/* 交易所 */
	Provider		varchar(20)		not null,	/* 分类提供商 */
	FirstIndustry	varchar(50)		not null,	/* 一级行业 */
	SecondIndustry	varchar(50)		not null,	/* 二级行业 */
	ThirdIndustry	varchar(50)		not null,	/* 三级行业 */
	InDate			varchar(8)		not null,	/* 入选日期 */
	OutDate			varchar(8)		not null,	/* 剔除日期 */
	primary key (SecuCode,Provider,InDate)
);

-- 指数成分股(IndexConstituents)
Create Table IndexConstituents(
	IndexCode		varchar(10)		not null,	/* 指数代码 */
	ExchangeOfIndex	varchar(20)		not null,	/* 交易所of指数 */
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	ExchangeOfSecu	varchar(20)		not null,	/* 交易所of证券 */
	InDate			varchar(8)		not null,	/* 入选日期 */
	OutDate			varchar(8)		not null,	/* 剔除日期 */
	primary key (IndexCode,SecuCode,InDate)
);

-- 指数成分权重(IndexConstituentsWeight)
Create Table IndexConstituentsWeight(
	tmDate		varchar(8)		not null,	/* 日期 */
	IndexCode varchar(10)		not null,	/* 指数代码 */
	SecuCode	varchar(10)		not null,	/* 证券代码 */
	Weight		numeric(20,4)	not null,	/* 权重 */
	primary key (tmDate,IndexCode,SecuCode)
);

-- 股指期货标准合约(IFStandardContract)
Create Table IFStandardContract(
	StandardContractCode	varchar(10)		not null,	/* 标准合约代码 */
	StandardContractName	varchar(20)		not null,	/* 标准合约名称 */
	UnderlyingIndexCode		varchar(10)		not null,	/* 标的指数代码 */
	UnderlyingIndexExchange	varchar(20)		not null,	/* 标的指数交易市场 */
	Exchange				varchar(20)		not null,	/* 合约交易市场 */
	Multiplier				numeric(20,4)	not null,	/* 合约乘数 */
	primary key (StandardContractCode)
);
insert into IFStandardContract values ('IF','沪深300股指期货合约','000300.SH','上交所','CFFEX',300)

-- 国债期货标准合约(TFStandardContract)
Create Table TFStandardContract(
	StandardContractCode	varchar(10)		not null,	/* 标准合约代码 */
	StandardContractName	varchar(20)		not null,	/* 标准合约名称 */
	ParValue				numeric(20,4)	not null,	/* 一手合约面值 */
	CouponRate				numeric(20,4)	not null,	/* 虚拟票面利率 */
	Exchange				varchar(20)		not null,	/* 交易所 */
	primary key (StandardContractCode)
);

-- 期货月合约（FutureContract）
Create Table FutureContract(
	StandardContractCode	varchar(10)		not null,	/* 标准合约代码 */
	ContractCode	varchar(10)		not null,	/* 合约代码 */
	ContractName	varchar(20)		not null,	/* 合约名称 */
	DeliveryMonth	varchar(6)		not null,	/* 交割月份 */
	Exchange		varchar(20)		not null,	/* 交易所 */
	Currency		varchar(10)		not null,	/* 交易货币 */
	ListedDate		varchar(8)		not null,	/* 开始交易日 */
	LastTradingDate	varchar(8)		not null,	/* 最后交易日 */
	LastDeliveryDate	varchar(8)	not null,	/* 最后交割日 */
	primary key (ContractCode)
);

-- 期货合约行情数据（FutureQuotationData）
Create Table FutureQuotationData(
	tmDate				varchar(8)		not null,	/* 日期 */
	ContractCode		varchar(10)		not null,	/* 合约代码 */
	PrevSettlement		numeric(20,4)	not null,	/* 前结算价 */
	Open				numeric(20,4)	not null,	/* 开盘价 */
	High				numeric(20,4)	not null,	/* 最高价 */
	Low					numeric(20,4)	not null,	/* 最低价 */
	Close				numeric(20,4)	not null,	/* 收盘价 */
	Settlement			numeric(20,4)	not null,	/* 结算价 */
	Volume				numeric(20,4)	not null,	/* 成交量（手） */
	Amount				numeric(20,4)	not null,	/* 成交额（元） */
	OpenInterest		numeric(20,4)	not null,	/* 持仓量（手） */
	primary key (tmDate,ContractCode)
);

-- 中金所每日结算会员成交持仓排名(CFFEXClearingMembersTradeData)
Create Table CFFEXClearingMembersTradeData(
	tmTradingDay		varchar(8)		not null,	/* 日期(tradingDay) */
	ContractCode		varchar(10)		not null,	/* 合约代码(instrumentId) */
	DataTypeID			int						not null,	/* 数据排名类型ID(dataTypeId,0=成交量排名,1=持买单量排名,2=持卖单量排名) */
	DataType				varchar(50)		not null,	/* 数据排名类型(成交量排名,持买单量排名,持卖单量排名) */
	Rank						int						not null,	/* 排名(rank) */
	MemberName			varchar(50)		not null,	/* 成员名称(shortname) */
	MemberID				varchar(10)		not null,	/* 成员ID(partyid) */
	Volume					numeric(20,4)	not null,	/* 数量(volume) */
	VolumeChg				numeric(20,4)	not null,	/* 增量(varVolume)*/
	StandardContractCode	varchar(10)	not null,	/* 标准合约代码(productid) */
	primary key (tmTradingDay,ContractCode,DataTypeID,MemberID)
);

/* 一手交易数量 */
Create Table RoundlotInfo(
	SecuType	varchar(20)		not null,	/* 证券类别（股票、股指期货、ETF、债券） */
	Roundlot	int				not null,	/* 一手交易数量 */
	primary key (SecuType)
);

-- 分级基金基本信息(StructuredFundInfo)
Create Table StructuredFundInfo(
	ParentFundCode		varchar(10)		not null,		/* 母基金代码 */
	ParentFundName		varchar(20)		not null,		/* 母基金名称 */
	BenchmarkIndexCode	varchar(10)		not null,		/* 基准指数代码 */
	FundDuration		int				not null,		/* 分级基金运作期，0 = 不定期 */
	tmEstablishDate		varchar(8)		not null,		/* 成立日 */
	tmListedDate		varchar(8)		not null,		/* 上市日 */
	Exchange			varchar(20)		not null,		/* 交易所 */
	AnchoringRate		varchar(8)		not null,		/* 锚定利率（优先级份额约定收益的基准利率名称） */
	ReturnAddition		numeric(20,4)	not null,		/* 收益加成 */
	ConvertDateType		int				not null,		/* 折算日期类型，0=无折算日期；1=会计年度；2=运作年度； */
	ConvertPeriod		numeric(20,4)	not null,		/* 折算周期（年） */
	ConvertTimesOneYear	int				not null,		/* 每年折算次数 */
	ConvertRule			int				not null,		/* 折算规则，0=不折算，1=A份额归1；2=三类份额归1 */
	UpperConvertPoint	varchar(10)		not null,		/* 上折算点（母基金份额净值超过上折算点时，三类份额净值归一），如果没有上折算点则为NA */
	LowerConvertPoint	varchar(10)		not null,		/* 下折算点（B份额净值低于下折算点时，三类份额净值归一），如果没有下折算点则为NA */
	AFundCode			varchar(10)		not null,		/* A份额代码 */
	AFundName			varchar(20)		not null,		/* A份额名称 */
	AFundVol			int				not null,		/* A份额配置数量 */
	BFundCode			varchar(10)		not null,		/* B份额代码 */
	BFundName			varchar(20)		not null,		/* B份额名称 */
	BFundVol			int				not null,		/* B份额配置数量 */
	primary key (ParentFundCode)
);

-- 分级基金分析指标表(StructuredFundAnalysisIndicator)
Create Table StructuredFundAnalysisIndicator(
	tmDate			varchar(8)		not null,	/* 日期 */
	ParentFundCode	varchar(10)		not null,	/* 母基金代码 */
	IndicatorName	varchar(50)		not null,	/* 指标名称（english name is prefered） */
	IndicatorValue	numeric(20,4)	not null,	/* 指标值 */
	primary key (tmDate,ParentFundCode,IndicatorName)
);

-- 股票分级基金套利信息高频表
Create Table StructuredFundArbitrageHFData(
	tmDate			varchar(8)		not null,	/* 日期(yyyyMMdd) */
	time			varchar(8)		not null,	/* 时间(hh:mm:ss) */
	BaseFundCode	varchar(10)		not null,	/* 母基金代码 */
	BaseFundName	varchar(20)		not null,	/* 母基金名称 */
	FundCompany		varchar(50)		null,		/* 基金公司名称 */
	tmIssueDate		varchar(8)		null,		/* 成立日期 */
	tmMaturityDate	varchar(8)		null,		/* 到期日期 */
	ManageFee		numeric(20,4)	null,		/* 管理费率% */
	IndexCode		varchar(10)		null,		/* 基准指数代码 */
	IndexName		varchar(20)		null,		/* 基准指数名称 */
	UpperRecalcPrice	numeric(20,4)	null,	/* 上折价格 */
	LowerRecalcPrice	numeric(20,4)	null,	/* 下折价格 */
	AssetRatio		numeric(20,4)	null,		/* 股票资产仓位上限% */
	AFundRatio		numeric(20,4)	null,		/* A份额的份数 */
	BFundRatio		numeric(20,4)	null,		/* B份额的份数 */
	FundDescr		varchar(100)	null,		/* 基金简述 */
	tmNextRecalcDate	varchar(8)	null,		/* 下一折算日期 */
	AFundCode		varchar(10)		not null,	/* A份额代码 */
	AFundName		varchar(20)		null,		/* A份额名称 */
	CouponDescr		varchar(50)		null,		/* A份额约定收益描述 */
	Coupon			numeric(20,4)	null,		/* 约定利率% */
	NextCoupon		numeric(20,4)	null,		/* 下一期约定利率% */
	BFundCode		varchar(10)		not null,	/* B份额代码 */
	BFundName		varchar(20)		null,		/* B份额名称 */
	LowerRecalcComment	varchar(50)	null,		/* 对低折的描述 */
	AFundAmount		numeric(20,4)	null,		/* A类基金总份额 */
	AFundAmountIncr	numeric(20,4)	null,		/* A类基金份额增长率 */
	StatusCD		varchar(20)		null,		/* 状态 */
	ApplyFee		varchar(50)		null,		/* 申购费率（描述） */
	ApplyFeeTip		varchar(200)	null,		/* 申购费率详述 */
	RedeemFee		varchar(50)		null,		/* 赎回费率（描述） */
	RedeemFeeTip	varchar(200)	null,		/* 赎回费率详述 */
	MinApplyAmount	numeric(20,4)	null,		/* 最小申购金额 */
	--Notes			varchar(100)	null,		/* 基金主页地址 */
	BaseFundNAV		numeric(20,4)	null,		/* 母基金净值 */
	BaseFundNAVDate	varchar(8)		null,		/* 母基金净值日期 */
	BFundNAVLeverage	numeric(20,4)	null,	/* B份额净值杠杆 */
	BaseFundLowerRecalcRate	numeric(20,4)	null,	/* 到达低折母基金需下跌% */
	BaseFundEstNAV	numeric(20,4)	null,		/* 母基金净值估计 */
	BaseFundEstPremium	numeric(20,4)	null,	/* 按AB份额现价计算的母基金整体折溢价率% */
	IndexIncrRate	numeric(20,4)	null,		/* 基准指数涨幅 */
	BFundEstNAV		numeric(20,4)	null,		/* B份额估计净值 */
	EstTime			varchar(8)		null,		/* 估算时间(hh:mm:ss) */
	BuyRedeemPremium	numeric(20,4)	null,	/* 按AB份额卖一价计算的整体折溢价率% */
	ApplySellPremium	numeric(20,4)	null,	/* 按AB份额买一价计算的整体折溢价率 */
	ABFundRate		varchar(8)		null,		/* AB份额间的比例 */
	AFundCurrentPrice	numeric(20,4)	null,	/* A份额最新价格 */
	AFundSell1Price		numeric(20,4)	null,	/* A份额卖一价 */
	AFundBuy1Price		numeric(20,4)	null,	/* A份额买一价 */
	AFundSell1Amount	numeric(20,4)	null,	/* A份额卖一价挂单量 */
	AFundBuy1Amount		numeric(20,4)	null,	/* A份额买一价挂单量 */
	AFundNAV			numeric(20,4)		null,	/* A份额净值 */
	tmAFundNAVDate		varchar(8)		null,	/* A份额的净值日期 */
	AFundTradeVolume	numeric(20,4)	null,	/* A份额当天的成交金额（万元） */
	BFundCurrentPrice		numeric(20,4)	null,	/*  B份额最新价格 */
	BFundSell1Price		numeric(20,4)	null,		/* B份额卖一价 */
	BFundBuy1Price		numeric(20,4)	null,		/* B份额买一价 */
	BFundSell1Amount	numeric(20,4)	null,	/* B份额卖一价的挂单量 */
	BFundBuy1Amount		numeric(20,4)	null,	/* B份额买一价的挂单量 */
	BFundNAV			numeric(20,4)	null,		/* B份额净值 */
	tmBFundNAVDate		varchar(8)	null,		/* B份额的净值日期 */
	BFundTradeVolume	numeric(20,4)	null,	/* B份额当天额成交金额（万元） */
	MergePrice			numeric(20,4)	null,	/* AB份额按照两者比例和现价合并计算的价格 */
	AFundPriceLastDate	varchar(8)	null,		/* A份额现价的更新日期 */
	AFundPriceLastTime	varchar(8)	null,		/* A份额现价的更新时间 */
	BFundPriceLastDate	varchar(8)	null,		/* B份额现价的更新日期 */
	BFundPriceLastTime	varchar(8)	null,		/* B份额现价的更新时间 */
	AFundAmountIncrease	numeric(20,4)	null,	/* A份额新增（万份） */
	BFundAmountIncrease	numeric(20,4)	null,	/* B份额新增（万份） */
	primary key (tmDate,time,BaseFundCode)
);

-- 股票分级基金A份额高频数据表
Create Table StructuredFundAFundHFData(
	tmDate		varchar(8)		not null,	/* 日期(yyyyMMdd) */
	time			varchar(8)		not null,	/* 时间(hh:mm:ss) */
	AFundCode	varchar(10)		not null,	/* A份额代码 */
	AFundName	varchar(20)		 null,	/* A份额名称 */
	LastTime		varchar(8)		null,	/* 最后更新时间 */
	AFundCurrentPrice	numeric(20,4)	 null,	/* A份额现价 */
	AFundNAVDate	varchar(8)		null,	/* 净值日期 */
	AFundNAV		numeric(20,4)	null,	/* A份额净值 */
	BaseFundCode	varchar(10)		not null,	/* 母基金代码 */
	AFundVolume		numeric(20,4)	null,	/* 成交额（万元） */
	IndexIncreaseRate	numeric(20,4)	null,	/* 基准指数涨幅% */
	LowerRecalcRate	numeric(20,4)	null,	/* 低折母基金需跌% */
	AFundPriceChange	numeric(20,4)	null,	/* A份额价格涨跌幅% */
	AFundYearsToMaturity	varchar(20)	null,	/* A份额剩余年限（年） */
	AFundDiscountRate	numeric(20,4)	null,	/* A份额折价率%(>0为折价，<0为溢价) */
	AFundCoupon		numeric(20,4)	null,	/* A份额本期约定收益率% */
	AFundNextCoupon	numeric(20,4)	null,	/* A份额下期约定收益率% */
	AFundProfitRate		numeric(20,4)	null,	/* A份额降息前收益率% */
	AFundModifiedProfitRate	numeric(20,4)	null,	/* A份修正收益率% */
	IndexCode		varchar(10)		null,	/* 母基金基准指数代码 */
	IndexName		varchar(20)		null,	/* 母基金基准指数名称 */
	ABRate			varchar(10)		null,	/* A:B */
	BaseFundEstDisRate	numeric(20,4)	null,	/* 整体溢价率% */
	FundDescr		varchar(100)	null,	/* 定期折算描述 */
	AFundAmount		numeric(20,4)	null,	/* A份额数量（万份） */
	AFundAmountIncreaseRate		numeric(20,4)	null,	/* A份额增幅% */
	primary key (tmDate,time,AFundCode)
);

-- 基金净值(FundNAV)
Create Table FundNAV(
	tmDate		varchar(8)		not null,	/* 日期 */
	FundCode	varchar(10)		not null,	/* 基金代码 */
	Exchange	varchar(20)		not null,	/* 交易所 */
	UnitNAV		numeric(20,4)	not null,	/* 单位净值 */
	AccuUnitNAV	numeric(20,4)	not null,	/* 累计单位净值 */
	ifExRight	int		not null,	/* 是否为除权日 */
	AccuUnitDividend	numeric(20,4)	not null,	/* 累计单位分红 */
	Factor		numeric(20,4)	not null,	/* 复权因子 */
	RUnitNAV	numeric(20,4)	not null,	/* 复权单位净值 */
	primary key (tmDate,FundCode)
);

-- 市场利率信息(MarketRateInfo)
Create Table MarketRateInfo(
	MarketRateCode		varchar(30)		not null,	/* 市场利率代码 */
	MarketRateName		varchar(50)		not null,	/* 市场利率名称 */
	primary key (MarketRateCode)
);


-- 市场利率数据(MarketRateData)
Create Table MarketRateData(
	tmDate						varchar(8)		not null,	/* 日期 */
	MarketRateCode		varchar(30)		not null,	/* 市场利率代码 */
	YieldType					varchar(10)		not null,	/* 收益率类型(Spot=即期,TYM=到期) */
	MarketRate				numeric(20,4)	not null,	/* 市场利率值 */
	primary key (tmDate,MarketRateCode,YieldType)
);

-- 配对交易股票对(PairTradingLegsInfo)
Create Table PairTradingLegsInfo(
	tmDate				varchar(8)		not null,	/* 日期 */
	Portfolio			varchar(50)		not null,	/* 组合名称 */
	Strategy			varchar(50)		not null,	/* 策略名称 */
	Leg1Code			varchar(10)		not null,	/* leg1的证券代码 */
	Leg1Name			varchar(20)		not null,	/* leg1的证券名称 */
	Leg1Exchange		varchar(20)		not null,	/* leg1的交易所 */
	Leg1Industry		varchar(50)		not null,	/* leg1的行业 */
	Leg1StochasticValue	int				not null,	/* leg1的stochastic indicator */
	Leg1ATR				numeric(20,4)	not null,	/* leg1的average true range */
	Leg1TradeVolume		numeric(20,4)	not null,	/* leg1的交易量,leg1默认为买入 */
	Leg2Code			varchar(10)		not null,	/* leg2的证券代码 */
	Leg2Name			varchar(20)		not null,	/* leg2的证券名称 */
	Leg2Exchange		varchar(20)		not null,	/* leg2的交易所 */
	Leg2Industry		varchar(50)		not null,	/* leg2的行业 */
	Leg2StochasticValue	int				not null,	/* leg2的stochastic indicator */
	Leg2ATR				numeric(20,4)	not null,	/* leg2的average true range */
	Leg2TradeVolume		numeric(20,4)	not null,	/* leg2的交易量,leg2默认为卖出 */
	StochasticDiff		int				not null,	/* the stochastic diff of leg1 and leg2 */
	CorrValue			numeric(20,4)	not null,	/* 收益率协方差 */
	hasExited			int				not null,	/* 是否已触发退出交易，1=已触发退出交易;0=未触发退出交易 */
	tmEntryDate			varchar(8)	not null,		/* 配对交易触发日期 */
	tmExitDate		varchar(8)		null,	/* 退出日期（执行退出交易的日期） */
	ProfitAndLoss	numeric(20,4)		null,	/* 收益 */
	primary key (tmDate,Portfolio,Strategy,Leg1Code,Leg2Code)
);

-- 策略持仓可用表(StrategyHoldingAvailable)
Create Table StrategyHoldingAvailable(
	tmDate				varchar(8)		not null,	/* 日期 */
	Portfolio			varchar(50)		not null,	/* 组合名称 */
	Strategy			varchar(50)		not null,	/* 策略名称 */
	SecuCode			varchar(10)		not null,	/* 证券代码（保证金=999999，现金=888888 */
	SecuName			varchar(20)		not null,	/* 证券简称（保证金=Margin，现金=Cash */
	SecuType			varchar(20)		not null,	/* 证券类别（股票、ETF、股指期货、保证金（用于计算衍生品的盈亏）、现金 */
	Exchange			varchar(20)		not null,	/* 交易所 */
	Industry			varchar(50)		not null,	/* 行业 */
	HoldingStatus		varchar(10)		not null,	/* 多空状态（多仓、空仓） */
	IsCreditTrade		int				not null,	/* 是否为信用交易，0=否；1=是 */
	Holding				numeric(20,4)	not null,	/* 持仓量（股票：股；期指：手；债券：张；保证金：元） */
	Available			numeric(20,4)	not null,	/* 可用量（与HoldingStatus对应：HoldingStatus为多仓，那么为多仓可用量；HoldingStatus为空仓，那么为空仓可用量） */
	ID						int						not null,	/* 持仓ID，同一天、同一组合、同一策略的持仓ID不重复 */
	primary key (tmDate,Portfolio,Strategy,SecuCode,HoldingStatus,IsCreditTrade)
);

-- 策略持仓生命周期表(StrategyHoldingLifecycle)
Create Table StrategyHoldingLifecycle(
	Porfolio		varchar(50)		not null,	/* 组合名称 */
	Strategy		varchar(50)		not null,	/* 策略名称 */
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	SecuName		varchar(20)		not null,	/* 证券简称 */
	SecuType		varchar(20)		not null,	/* 证券类别：股票、ETF、股指期货 */
	Exchange		varchar(20)		not null,	/* 交易所 */
	Industry		varchar(50)		not null,	/* 行业 */
	HoldingStatus	varchar(10)	not null,	/* 多空状态：多仓、空仓 */
	IsCreditTrade	int					not null,	/* 是否为信用交易：0=否;1=是 */
	tmInDate		varchar(8)		not null,	/* 入选日期 */
	tmOutDate		varchar(8)		not null	/* 剔除日期 */
);

-- 中金所结算会员情绪指标
Create Table CFFEXMemberSentimentInfo(
	tmTradingDay		varchar(8)		not null,	/* 交易日期 */
	ContractCode		varchar(10)		not null,	/* 合约代码 */
	MemberID			varchar(10)		not null,	/* 结算成员ID，特殊成员=9999 */
	MemberName			varchar(50)		not null,	/* 成员名称 */
	TradeVolume			numeric(20,4)	not null,	/* 成交量 */
	LongPosition		numeric(20,4)	not null,	/* 持买单量 */
	ShortPosition		numeric(20,4)	not null,	/* 持卖单量 */
	SentimentValue		numeric(20,4)	not null,	/* 情绪值 */
	StandardContractCode	varchar(10)	not null,	/* 标准合约代码 */
	primary key (tmTradingDay,ContractCode,MemberID)
);

-- 期货知情交易者情绪指标
Create Table FutureInformedTraderSentimentInd(
	tmTradingDay		varchar(8)		not null,	/* 交易日期 */
	ContractCode		varchar(10)		not null,	/* 合约代码 */
	SentimentCode		varchar(10)		not null,	/* 情绪指标代码 */
	SentimentValue		numeric(20,4)	not null,	/* 情绪指标值 */
	primary key (tmTradingDay,ContractCode,SentimentCode)
);

-- 非金融类资产负债表
Create Table BalanceSheetData(
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	tmPubDate	varchar(8)		not null,	/* 公告日期 */
	tmReportDate	varchar(8)	not null,	/* 报告日期（截止日期） */
	mark	int	not null,	/* 合并调整标志，1=合并调整,2=合并未调整,3=母公司调整,4=母公司未调整 */
	Inventories	numeric(20,4)	null,	/* 存货 */
	TotalCurrentAssets	numeric(20,4)	null,	/* 流动资产合计 */
	TotalNonCurrentAssets	numeric(20,4)	null,	/* 非流动资产合计 */
	TotalAssets	numeric(20,4)	null,	/* 资产总计 */
	TotalCurrentLiability	numeric(20,4)	null,	/* 流动负债合计 */
	TotalNonCurrentLiability	numeric(20,4)	null,	/* 非流动负债合计 */
	TotalLiability	numeric(20,4)	null,	/* 负债合计 */
	PaidInCapital	numeric(20,4)	null, 	/* 实收资本（或股本）*/
  CapitalReserveFund	numeric(20,4)	null, 	/* 资本公积 */
  SurplusReserveFund	numeric(20,4)	null, 	/* 盈余公积 */
  RetainedProfit	numeric(20,4)	null,	/* 未分配利润 */
  SEWithoutMI	numeric(20,4)	null,	/* 归属母公司股东权益合计 */
  MinorityInterests	numeric(20,4)	null,	/* 少数股东权益 */
  TotalShareholderEquity	numeric(20,4)	null,	/* 所有者权益（或股东权益）合计 */
  TotalLiabilityAndEquity	numeric(20,4)	null,	/* 负债和所有者权益（或股东权益合计）*/
  primary key (SecuCode,tmReportDate,mark)
);

-- 资产负债表
Create Table BalanceSheetData(
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	tmPubDate		varchar(8)		not null,	/* 公告日期 */
	tmReportDate		varchar(8)	not null,	/* 报告日期（截止日期） */
	ReportType		varchar(20)		not null,	/* 报表类型:合并报表,合并报表(调整),母公司报表,母公司报表(调整) */
	CompanyType		int		not null,					/* 1=非金融类,2=银行,3=保险,4=证券 */
	CashEquivalents		numeric(20,4)	null,	/* 货币资金 */
	TradingAssets			numeric(20,4)	null,	/* 交易性金融资产 */
	BillReceivable		numeric(20,4)	null,	/* 应收票据 */
	AccountReceivable	numeric(20,4)	null,	/* 应收账款 */
	OtherReceivable		numeric(20,4)	null,	/* 其他应收款 */
	AdvancePayment		numeric(20,4) null,	/* 预付款项 */
	DividendReceivable	numeric(20,4)	null,	/* 应收股利 */
	InterestReceivable	numeric(20,4)	null,	/* 应收利息 */
	Inventories		numeric(20,4)	null,	/* 存货 */
	DeferredExpense		numeric(20,4)	null,	/* 待摊费用 */
	TotalCurrentAssets	numeric(20,4)	null,	/* 流动资产合计 */
	HoldForSaleAssets		numeric(20,4)	null,	/* 可供出售金融资产 */
	HoldToMaturityInvestments	numeric(20,4)	null,	/* 持有至到期投资 */
	LongtermEquityInvest	numeric(20,4)	null,	/* 长期股权投资 */
	LongtermReceivableAccount	numeric(20,4)	null,	/* 长期应收款 */
	FixedAssets		numeric(20,4)	null,	/* 固定资产 */
	IntangibleAssets	numeric(20,4)	null,	/* 无形资产 */
	GoodWill			numeric(20,4)	null,	/* 商誉 */
	LongDeferredExpense	numeric(20,4)	null,	/* 长期待摊费用 */
	TotalNonCurrentAssets	numeric(20,4) null,	/* 非流动资产合计 */
	TotalAssets		numeric(20,4)	null,	/* 资产总计 */
	ShortTermLoan	numeric(20,4)	null,	/* 短期借款 */
	TradingLiability	numeric(20,4) null,	/* 交易性金融负债 */
	NotesPayable	numeric(20,4)	null,	/* 应付票据 */
	AccountsPayable		numeric(20,4)	null,	/* 应付账款 */
	AdvanceReceipts		numeric(20,4)	null,	/* 预收款项 */
	TaxsPayable		numeric(20,4)	null,	/* 应交税费 */
	InterestPayable		numeric(20,4)	null,	/* 应付利息 */
	DividendPayable		numeric(20,4)	null,	/* 应付股利 */
	AccruedExpense		numeric(20,4)	null,	/* 预提费用 */
	TotalCurrentLiability	numeric(20,4)	null,	/* 流动负债合计 */
	LongtermLoan			numeric(20,4)	null,	/* 长期借款 */
	LongtermAccountPayable	numeric(20,4)	null,	/* 应付债券 */
	TotalNonCurrentLiability	numeric(20,4)	null,	/* 非流动负债合计 */
	TotalLiability		numeric(20,4)	null,	/* 负债合计 */
	PaidInCapital			numeric(20,4)	null,	/* 实收资本（或股本） */
	CapitalReserveFund		numeric(20,4) null,	/* 资本公积 */
	SurplusReserveFund		numeric(20,4)	null,	/* 盈余公积 */
	RetainedProfit		numeric(20,4)	null,	/* 未分配利润 */
	MinorityInterests			numeric(20,4)	null,	/* 少数股东权益 */
	SEWithoutMI				numeric(20,4)	null,	/* 归属母公司股东权益 */
	TotalShareholderEquity		numeric(20,4)	null,	/* 所有者权益（或股东权益） */
	TotalLiabilityAndEquity		numeric(20,4)	null	/* 负债和所有者权益（或股东权益）总计 */
);

-- 利润表
Create Table IncomeStatementData(
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	tmPubDate		varchar(8)		not null,	/* 公告日期 */
	tmReportDate		varchar(8)	not null,	/* 报告日期（截止日期） */
	ReportType		varchar(20)		not null,	/* 报表类型:合并报表,合并报表(调整),母公司报表,母公司报表(调整) */
	CompanyType		int		not null,					/* 1=非金融类,2=银行,3=保险,4=证券 */
	TotalOperatingRevenue		numeric(20,4)	null,	/* 营业总收入 */
	OperatingRevenue		numeric(20,4)	null,	/* 营业收入 */
	NetInterestIncome		numeric(20,4)	null,	/* 利息净收入 */
	NetCommissionIncome	numeric(20,4)	null,	/* 手续费及佣金净收入 */
	PremiumsEarned			numeric(20,4)	null,	/* 已赚保费 */
	OtherOperatingRevenue		numeric(20,4)	null,	/* 其他营业收入 */
	TotalOperatingCost	numeric(20,4)	null,	/* 营业总成本 */
	OperatingPayout			numeric(20,4)	null,	/* 营业支出 */
	OperatingCost		numeric(20,4)	null,		/* 营业成本 */
	OperatingTaxSurcharges	numeric(20,4)	null,	/* 营业税金及附加 */
	OperatingExpense		numeric(20,4)	null,	/* 销售费用 */
	AdministrationExpense	numeric(20,4)	null,	/* 管理费用 */
	FinancialExpense		numeric(20,4)	null,	/* 财务费用 */
	FairValueChangeIncome		numeric(20,4)	null,	/* 公允价值变动净收益 */
	InvestIncome		numeric(20,4)	null,	/* 投资净收益 */
	ExchangeIncome	numeric(20,4)	null,	/* 汇兑收益 */
	OperatingProfit	numeric(20,4)	null,	/* 营业利润 */
	NonoperatingIncome	numeric(20,4)	null,	/* 营业外收入 */
	NonoperatingExpense	numeric(20,4)	null,	/* 营业外支出 */
	TotalProfit			numeric(20,4)	null,	/* 利润总额 */
	IncomeTaxCost		numeric(20,4)	null,	/* 所得税费用 */
	NetProfit				numeric(20,4)	null,	/* 净利润 */
	NPParentCompanyOwners		numeric(20,4)	null,	/* 归属于母公司所有者的净利润 */
	MinorityProfit	numeric(20,4)	null,	/* 少数股东损益 */
	BasicEPS			numeric(20,4)	null,	/* 基本每股收益 */
	DilutedEPS		numeric(20,4)	null	/* 稀释每股收益 */
);

-- 现金流量表
Create Table CashFlowStatementData(
	SecuCode		varchar(10)		not null,	/* 证券代码 */
	tmPubDate		varchar(8)		not null,	/* 公告日期 */
	tmReportDate		varchar(8)	not null,	/* 报告日期（截止日期） */
	ReportType		varchar(20)		not null,	/* 报表类型:合并报表,合并报表(调整),母公司报表,母公司报表(调整) */
	CompanyType		int		not null,					/* 1=非金融类,2=银行,3=保险,4=证券 */
	SubtotalOperateCashInflow	numeric(20,4)	null,	/* 经营活动现金流入小计 */
	SubtotalOperateCashOutflow	numeric(20,4)	null,	/* 经营活动现金流出小计 */
	NetOperateCashFlow		numeric(20,4)	null,	/* 经营活动产生的现金流量净额 */
	SubtotalInvestCashInflow		numeric(20,4)	null,	/* 投资活动现金流入小计 */
	SubtotalInvestCashOutflow		numeric(20,4)	null,	/* 投资活动现金流出小计 */
	NetInvestCashFlow		numeric(20,4)	null,	/* 投资活动产生的现金流量净额 */
	SubtotalFinanceCashInflow		numeric(20,4)	null,	/* 筹资活动现金流入小计 */
	SubtotalFinanceCashOutflow	numeric(20,4)	null,	/* 筹资活动现金流出小计 */
	NetFinanceCashFlow	numeric(20,4)	null,	/* 筹资活动产生的现金流量净额 */
	CashEquivalentIncrease	numeric(20,4)	null,	/* 现金及现金等价物净增加额 */
	BeginPeriodCash		numeric(20,4)	null,	/* 加：期初现金及现金等价物余额 */
	EndPeriodCashEquivalent	numeric(20,4)	null,	/* 期末现金及现金等价物余额 */
	NetProfit			numeric(20,4)	null,	/* 净利润 */
	AssetsDepreciationReserves	numeric(20,4)	null,	/* 加:资产减值准备 */
	FixedAssetDepreciation	numeric(20,4)	null,	/* 固定资产折旧 */
	IntangibleAssetAmortization	numeric(20,4)	null,	/* 无形资产摊销 */
	DeferredExpenseAmort		numeric(20,4)	null,	/* 长期待摊费用摊销 */
	DeferredExpenseDecreased		numeric(20,4)	null,	/* 待摊费用减少(减:增加) */
	AccruedExpenseAdded			numeric(20,4)	null	/* 预提费用增加(减:减少) */
);

-- 金融类资产负债表
Create Table FBalanceSheetData(
	SecuCode	varchar(10)		not null,	/* 证券代码 */
  tmPubDate		varchar(8)	not null,	/* 公告日期 */
  tmReportDate	varchar(8)	not null,	/* 报告日期（截止日期） */
  mark	int		not null,	/* 合并调整标志,1=合并调整,2=合并未调整,3=母公司调整,4=母公司未调整 */
  EnterpriseType	int not null,	/* 企业性质,13=商业银行,31=证券公司,33=信托公司,35=保险公司 */
  TotalAssets		numeric(20,4)	null,	/* 资产总计 */
  TotalLiability	numeric(20,4)	null,	/* 负债合计 */
  PaidInCapital	numeric(20,4)	null,	/* 实收资本（或股本） */
  CapitalReserveFund	numeric(20,4)	null,	/* 资本公积 */
  SurplusReserveFund	numeric(20,4)	null,	/* 盈余公积 */
  RetainedProfit	numeric(20,4)	null,	/* 未分配利润 */
  SEWithoutMI	numeric(20,4)	null,	/* 归属母公司股东权益合计 */
  MinorityInterests	numeric(20,4)	null,	/* 少数股东权益 */
  TotalShareholderEquity	numeric(20,4)	null,	/* 所有者权益（或股东权益）合计 */
  TotalLiabilityAndEquity	numeric(20,4)	null,	/* 负债和所有者权益（或股东权益）合计 */
  primary key (SecuCode,tmReportDate,mark)
);

-- 非金融类利润表
Create Table IncomeStatementData(
    SecuCode	varchar(10)		not null,	/* 证券代码 */
    tmPubDate	varchar(8)	not null,	/* 公告日期 */
    tmReportDate	varchar(8)	not null,	/* 报告日期（截止日期） */
    mark	int	not null,	/* 1=合并报告期调整,2=合并报告期未调整,3=母公司报告期调整,4=母公司报告期未调整,5=合并季度调整,6=合并季度未调整,7=母公司季度调整,8=母公司季度未调整 */
    TotalOperatingRevenue	numeric(20,4)	 null,	/* 营业总收入 */
    OperatingRevenue	numeric(20,4)		null,	/* 其中：营业收入 */
    OtherOperatingRevenue	numeric(20,4)	 null,	/* 其他营业收入 */
    TotalOperatingCost	numeric(20,4)	 null,	/* 营业总成本 */
    OperatingCost	numeric(20,4)	 null,	/* 其中：营业成本 */
    OtherOperatingCost	numeric(20,4)	 null,	/* 其他营业成本 */
    OperatingTaxSurcharges	numeric(20,4)	 null,	/* 营业税金及附加 */
    OperatingExpense	numeric(20,4)	 null,	/* 销售费用 */
    AdministrationExpense	numeric(20,4)	 null,	/* 管理费用 */
    FinancialExpense	numeric(20,4)	 null,	/* 财务费用 */
    AssetImpairmentLoss	numeric(20,4)	 null,	/* 资产减值损失 */
    OtherNetRevenue	numeric(20,4)	 null,	/* 非经营性净收益 */
    FairValueChangeIncome	numeric(20,4)	 null,	/* 公允价值变动净收益 */
    InvestIncome	numeric(20,4)	 null,	/* 投资净收益 */
    ExchangeIncome	numeric(20,4)	 null,	/* 汇兑收益 */
    OperatingProfit	numeric(20,4)	 null,	/* 营业利润 */
    NonoperatingIncome	numeric(20,4)	 null,	/* 加：营业外收入 */
    NonoperatingExpense	numeric(20,4)	 null,	/*减：营业外支出 */
    TotalProfit	numeric(20,4)	 null,	/* 利润总额 */
    IncomeTaxCost	numeric(20,4)	 null,	/* 减：所得税费用 */
    NetProfit	numeric(20,4)	 null,	/* 净利润 */
    NPParentCompanyOwners	numeric(20,4)	 null,	/* 归属于母公司所有者的净利润 */
    MinorityProfit	numeric(20,4)	 null,	/* 少数股东权益 */
    OtherCompositeIncome	numeric(20,4)	 null,	/*其他综合收益 */
    TotalCompositeIncome	numeric(20,4)	 null,	/* 综合收益总额 */
    CIParentCompanyOwners	numeric(20,4)	 null,	/* 归属于母公司所有者的综合收益总额 */
    CIMinorityOwners	numeric(20,4)	 null,	/* 归属于少数股东的综合收益总额 */
    BasicEPS	numeric(20,4)	 null,	/* 基本每股收益 */
    DilutedEPS	numeric(20,4)	 null,	/* 稀释每股收益 */
    primary key (SecuCode,tmReportDate,mark)
);

-- 金融类利润表
Create Table FIncomeStatementData(
    SecuCode	varchar(10)		not null,	/* 证券代码 */
    tmPubDate	varchar(8)	not null,	/* 公告日期 */
    tmReportDate	varchar(8)	not null,	/* 报告日期（截止日期） */
    mark	int		not null,	/* 1=合并报告期调整,2=合并报告期未调整,3=母公司报告期调整,4=母公司报告期未调整,5=合并季度调整,6=合并季度未调整,7=母公司季度调整,8=母公司季度未调整 */
    EnterpriseType	int not null,	/* 企业性质,13=商业银行,31=证券公司,33=信托公司,35=保险公司 */
    OperatingRevenue	numeric(20,4)	null,	/* 营业收入 */
    NetInterestIncome	numeric(20,4)	null,	/* 利息净收入 */
    NetCommissionIncome	numeric(20,4)	null,	/* 手续费及佣金净收入*/
    PremiumsEarned	numeric(20,4)	null,	/* 已赚保费 */
    InvestIncome	numeric(20,4)	null,	/* 投资净收益 */
    OperatingPayout	numeric(20,4)	null,	/* 营业支出 */
    OperatingTaxAndSurcharges	numeric(20,4)	null,	/* 营业税金及附加 */
    OperatingProfit	numeric(20,4)	null,	/* 营业利润 */
    NonoperatingIncome	numeric(20,4)	null,	/* 加：营业外收入 */
    NonoperatingExpense	numeric(20,4)	null,	/* 减：营业外支出 */
    TotalProfit	numeric(20,4)	null,	/* 利润总额 */
    IncomeTaxCost	numeric(20,4)	null,	/*减：所得税 */
    NetProfit	numeric(20,4)	null,	/* 净利润 */
    NPFromParentCompanyOwners	numeric(20,4)	null,	/* 归属于母公司所有者的净利润 */
    MinorityProfit	numeric(20,4)	null,	/* 少数股东权益 */
    OtherCompositeIncome	numeric(20,4)	null,	/* 其他综合收益 */
    TotalCompositeIncome	numeric(20,4)	null,	/* 综合收益总额 */
    CIParentCompanyOwners	numeric(20,4)	null,	/* 归属于母公司所有者的综合收益总额 */
    CIMinorityOwners	numeric(20,4)	null,	/* 归属于少数股东的综合收益总额 */
    BasicEPS	numeric(20,4)	null,	/* 基本每股收益 */
    DilutedEPS	numeric(20,4)	null,	/* 稀释每股收益 */
    primary key (SecuCode,tmReportDate,mark)
);

-- 非金融类现金流量表
Create Table CashFlowStatementData(
    SecuCode	varchar(10)		not null,	/* 证券代码 */
    tmPubDate	varchar(8)	not null,	/* 公告日期 */
    tmReportDate	varchar(8)	not null,	/* 报告日期(截止日期) */
    mark	int	not null,	/* 1=合并调整,2=合并未调整,3=母公司调整,4=母公司未调整 */
    SubtotalOperateCashInflow		numeric(20,4)		null,	/* 经营活动现金流入小计 */
    SubtotalOperateCashOutflow		numeric(20,4)		null,	/* 经营活动现金流出小计 */
    NetOperateCashFlow		numeric(20,4)		null,	/* 经营活动产生的现金流量金额 */
    SubtotalInvestCashInflow		numeric(20,4)		null,	/* 投资活动现金流入小计 */
    SubtotalInvestCashOutflow		numeric(20,4)		null,	/* 投资活动现金流出小计 */
    NetInvestCashFLow		numeric(20,4)		null,	/* 投资活动产生的现金流量金额 */
    SubtotalFinanceCashInflow		numeric(20,4)		null,	/* 筹资活动现金流入小计 */
    SubtotalFinanceCashOutflow		numeric(20,4)		null,	/* 筹资活动现金流出小计 */
    NetFinanceCashFlow		numeric(20,4)		null,	/* 筹资活动产生的现金流量金额 */
    CashEquivalentIncrease		numeric(20,4)		null,	/* 现金及现金等价物净增加额 */
    BeginPeriodCash		numeric(20,4)		null,	/* 加:期初现金及现金等价物余额 */
    EndPeriodCashEquivalent		numeric(20,4)		null,	/* 期末现金及现金等价物余额 */
    NetProfit		numeric(20,4)		null,	/* 净利润 */
    AssetsDepreciationReserves		numeric(20,4)		null,	/* 加:资产减值准备 */
    FixedAssetDepreciation		numeric(20,4)		null,	/* 固定资产折旧 */
    IntangibleAssetAmortization		numeric(20,4)		null,	/* 无形资产摊销 */
    DeferredExpenseAmort		numeric(20,4)		null,	/*长期待摊费用摊销 */
    DeferredExpenseDecreased		numeric(20,4)		null,	/*待摊费用减少(减:增加) */
    AccruedExpenseAdded		numeric(20,4)		null,	/* 预提费用增加(减:减少) */
    primary key (SecuCode,tmReportDate,mark)
);

-- 金融类现金流量表
Create Table FCashFlowStatementData(
    SecuCode		varchar(10)		not null,	/* 证券代码 */
    tmPubDate	varchar(8)	not null,	/* 公告日期 */
    tmReportDate	varchar(8)	not null,	/* 报告日期（截止日期） */
    mark	int not null,	/* 1=合并调整,2=合并未调整,3=母公司调整,4=母公司未调整 */
    EnterpriseType	int	not null,	/* 企业性质,13=商业银行,31=证券公司,33=信托公司,35=保险公司 */
    SubtotalOperateCashInflow		numeric(20,4)	null,	/* 经营活动现金流入小计 */
    SubtotalOperateCashOutflow		numeric(20,4)	null,	/* 经营活动现金流出小计 */
    NetOperateCashFlow		numeric(20,4)	null,	/* 经营活动产生的现金流量净额 */
    SubtotalInvestCashInflow		numeric(20,4)	null,	/* 投资活动现金流入小计 */
    SubtotalInvestCashOutflow		numeric(20,4)	null,	/* 投资活动现金流出小计 */
    NetInvestCashFlow		numeric(20,4)	null,	/* 投资活动产生的现金流量净额 */
    SubtotalFinanceCashInflow		numeric(20,4)	null,	/* 筹资活动现金流入小计 */
    SubtotalFinanceCashOutflow		numeric(20,4)	null,	/* 筹资活动现金流出小计 */
    NetFinanceCashFlow		numeric(20,4)	null,	/* 筹资活动产生的现金流量净额 */
    CashEquivalentIncrease		numeric(20,4)	null,	/* 现金及现金等价物净增加额 */
    BeginPeriodCash		numeric(20,4)	null,	/* 加：期初现金及现金等价物余额 */
    EndPeriodCashEquivalent		numeric(20,4)	null,	/* 期末现金及现金等价物余额 */
    NetProfit		numeric(20,4)	null,	/* 净利润 */
    AssetsDepreciationReserves		numeric(20,4)	null,	/* 加:资产减值准备 */
    FixedAssetDepreciation		numeric(20,4)	null,	/*固定资产折旧 */
    IntangibleAssetAmortization		numeric(20,4)	null,	/* 无形资产摊销 */
    DeferredExpenseAmort		numeric(20,4)	null,	/* 长期待摊费用摊销 */
    DeferredExpenseDecreased		numeric(20,4)	null,	/* 待摊费用减少(减:增加) */
    AccruedExpenseAdded		numeric(20,4)	null,	/* 预提费用增加(减:减少) */
    primary key (SecuCode,tmReportDate,mark)
);

------------------ ETF停牌套利策略相关数据表------------------------
-- ETF清单
Create Table ETFList(
	ETFCode		varchar(10)		not null,	/* ETF代码 */
	ETFName		varchar(50)		not null,	/* ETF名称 */
	primary key (ETFCode)
);

-- ETF净值数据
Create Table ETFNAV(
	tmDate		varchar(8)	not null,	/* 日期 */
	ETFCode		varchar(10)	not null,	/* ETF代码 */
	TotalUnits	numeric(20,4)	not null,	/* ETF份额 */
	UnitNAV		numeric(20,4)	not null,	/* 单位净值 */
	NAV			numeric(20,4)	not null,	/* 净值 */
	primary key (tmDate,ETFCode)
);

-- ETF申赎清单
Create Table ETFConstituent(
	tmDate		varchar(8)	not null,	/* 日期 */
	ETFCode		varchar(10)	not null,	/* ETF代码 */
	SecuCode	varchar(10)	not null,	/* 证券代码 */
	SecuName	varchar(20)	not null,	/* 证券简称 */
	VolumeInConstituent	numeric(20,4)	not null,	/* 申赎清单中的数量 */
	Mark		varchar(10)	not null,	/* 现金替代标志(禁止，允许，必须，深市退补，深市必须) */
	PremiumRatio	numeric(20,4)	not null,	/* 现金替代溢价比例 */
	FixedSubstitutionAmount	numeric(20,4)	not null,	/* 固定替代金额 */
	PrevClose	numeric(20,4)	not null,	/* 前收盘价 */
	MarketValue	numeric(20,4)	not null,	/* 证券在申赎清单中的市值 */
	Weight		numeric(20,4)	not null,	/* 权重 */
	VolumeInETF	numeric(20,4)	not null,	/* ETF中的数量（估计） */
	primary key (tmDate,ETFCode,SecuCode)
);

-- 停牌数据
Create Table SuspendData(
	tmDate		varchar(8)		not null,	/* 日期 */
	SecuCode	varchar(10)		not null,	/* 证券代码 */
	SecuName	varchar(20)		not null,	/* 证券简称 */
	SuspendDate	varchar(8)		not null,	/* 停牌日期 */
	ResumptionDate	varchar(8)	null,		/* 复牌日期 */
	SuspendDuration	int			not null,	/* 停牌天数（交易日） */
	SuspendReason	varchar(100)	not null,	/* 停牌原因 */
	PreSuspendClose	numeric(20,4)	not null,	/* 停牌前收盘价 */
	primary key (tmDate,SecuCode)
);

-- ETF停牌股套利情况表
Create Table ETFSuspendArbInfo(
	tmDate		varchar(8)		not null,	/* 日期 */
	SecuCode	varchar(10)		not null,	/* 证券代码 */
	SecuName	varchar(20)		not null,	/* 证券简称 */
	SuspendDate	varchar(8)		not null,	/* 停牌日期 */
	ResumptionDate	varchar(8)	null,		/* 复牌日期 */
	SuspendDuration	int			not null,	/* 停牌天数（交易日） */
	SuspendReason	varchar(100)	not null,	/* 停牌原因 */
	PrevSuspendClose	numeric(20,4)	not null,	/* 停牌前收盘价 */
	UnderlyingETFs	varchar(200)	not null,	/* 标的ETF */
	ArbVolume	numeric(20,4)	not null,	/* 可套取数量（股） */
	Industry	varchar(50)		not null,	/* 行业（申万一级行业） */
	ImpliedRet	numeric(20,4)	not null,	/* 隐含涨幅（停牌至今上证综指涨幅） */
	Beta		numeric(20,4)	not null,	/* Beta */
	Close		numeric(20,4)	not null,	/* 最新收盘价 */
	ArbAmount	numeric(20,4)	not null,	/* 可套取市值（元） */
	HedgeAmount numeric(20,4)	not null,	/* 需要对冲市值（元） */
	Weight		numeric(20,4)	not null,	/* 权重 */
	primary key (tmDate,SecuCode)
);

-- ETF停牌股套利市场容量数据
Create Table ETFSuspendArbCapacity(
	tmDate		varchar(8)		not null,	/* 日期 */
	ETFTotalAmount	numeric(20,4)	not null,	/* ETF总额 */
	TotalSuspendAmount	numeric(20,4)	not null,	/* 停牌股总额 */
	ArbedAmount		numeric(20,4)	not null,	/* 停牌股被套利金额 */
	primary key (tmDate)
);

-- 复牌信息
Create Table ResumptionInfo(
	tmResumptionDate	varchar(8)	not null,	/* 复牌日期 */
	SecuCode	varchar(10)		not null,	/* 证券代码 */
);

-- 流动性差的ETF清单
Create Table PoorLiquidityETF(
	tmDate		varchar(8)		not null,	/* 日期 */
	ETFCode		varchar(10)		not null,	/* ETF代码 */
	primary key (tmDate,ETFCode)
);

-------------多因子模型相关表-----------------------------------------
-- 多因子模型指标信息表(指标库）
Create Table IndicatorInfo(
	ModelCode		varchar(20)		not null,		/* 模型代码 */
	FactorCode		varchar(20)		not null,		/* 因子代码 */
	IndicatorCode		varchar(20)		not null,		/* 指标代码 */
	IndicatorName		varchar(50)		not null,		/* 指标名称 */
	--IndicatorIndex	int		not null,		/* 指标序号，如换手率指标，一个月换手率、三个月换手率、六个月换手率等指标序号分别为0、1、2 */
	IntervalNum		int		not null,		/* 计算指标值时所采用的时间区间数量 */
	IntervalType		int		not null,		/* 计算指标值时所采用的时间区间类型,1=DAILY, 2=WEEKLY,  3=DOUBLEWEEKLY, 4=MONTHLY, 5=QUARTERLY,  6=SEMIANNUALLY, 7=ANNUALLY*/
	AlternativeParam		int		not null,		/*备用参数  */
	BenchmarkCode	varchar(10)		not null,	/* 计算指标值时的基准证券代码（如计算Beta时的基准指数） */
	Description		varchar(200)		null,		/* 指标描述 */
	IndicatorClassCode	varchar(20)		not null,	/* 指标类的代码,用于创建指标类 */
	primary key (ModelCode,IndicatorCode)
);

-- 证券指标值
Create Table SecuIndicatorValue(
	tmDate		varchar(8)		not null,		/* 日期 */
	ModelCode	varchar(20)	not null,		/* 模型代码 */
	SecuCode	varchar(10)		not null,		/* 证券代码 */
	IndicatorCode		varchar(20)		not null,		/* 指标代码 */
	--IndicatorIndex	int		not null,	/* 指标序号 */
	IndicatorValue	numeric(20,4)		null,	/* 指标值 */
	ifStandardised	int		not null,	/* 是否为标准化数据，0=没有标准化；1=已标准化 */
	primary key (tmDate,ModelCode,SecuCode,IndicatorCode,ifStandardised)
);

-- 多因子模型基本信息表
Create Table MFModelInfo(
	ModelCode		varchar(20)		not null,		/* 模型代码 */
	ModelName		varchar(20)		not null	/* 模型名称 */
);

-- 风险因子基本信息表
Create Table NormalFactorInfo(
	ModelCode		varchar(20)		not null,		/* 模型代码 */
	FactorCode	varchar(20)		not null,		/* 因子代码 */
	FactorName	varchar(50)	not null,		/* 因子名称 */
	primary key (ModelCode,FactorCode)
);

-- 因子构成
Create Table NormalFactorComposition(
	ModelCode		varchar(20)		not null,		/* 模型代码 */
	FactorCode	varchar(20)		not null,		/* 因子代码 */
	IndicatorCode	varchar(20)		not null,		/* 指标代码 */
	--IndicatorIndex	int		not nulll,		/* 指标序号 */
	InDate		varchar(8)	not null,		/* 调入日期 */
	OutDate	varchar(8)	null,		/* 剔除日期 */
	primary key (ModelCode,FactorCode,IndicatorCode,InDate)
);

-- 因子构成权重信息
Create Table NormalFactorWeightInfo(
	tmDate	varchar(8)		not null,	/* 日期 */
	ModelCode	varchar(20)		not null,	/* 模型代码 */
	FactorCode	varchar(20)		not null,	/* 因子代码 */
	IndicatorCode	varchar(20)		not null,	/* 指标代码 */
	--IndicatorIndex	int		not null,	/* 指标序号*/
	IndicatorWeight		numeric(20,4)	not null,	/* 指标权重 */
	primary key (tmDate,ModelCode,FactorCode,IndicatorCode)
);

-- 行业因子信息
Create Table IndustryFactorInfo(
	ModelCode	varchar(20)		not null,	/* 模型代码 */
	FactorCode	varchar(20)		not null,	/* 行业因子代码 */
	FactorName	varchar(50)		not null,  /* 行业因子名称 */
	IndustryCode	varchar(50)	not null,	/* 行业分类代码（一级行业） */
	Provider		varchar(20)		not null,	/* 行业分类提供商 */
	primary key (ModelCode,FactorCode)
);

-- 因子报酬	(包含行业因子）
Create Table FactorReturn(
	tmDate		varchar(8)		not null,	/* 日期 */
	ModelCode	varchar(20)		not null,	/* 模型代码 */
	FactorCode	varchar(20)		not null,	/* 因子代码 */
	FactorReturn	numeric(20,4)	not null,	/* 因子报酬 */
	primary key (tmDate,ModelCode,FactorCode)
);

-- 因子风险暴露（包含行业因子）
Create Table FactorExposure(
	tmDate		varchar(8)		not null,	/* 日期 */
	SecuCode	varchar(10)		not null,	/* 证券代码 */
	ModelCode	varchar(20)		not null,	/* 模型代码 */
	FactorCode	varchar(20)		not null,	/* 因子代码 */
	FactorExposure	numeric(20,4)	not null,	/* 因子风险暴露 */
	primary key (tmDate,SecuCode,ModelCode,FactorCode)
);

-- 指标检验数据
Create Table IndicatorTestData(
	tmDate		varchar(8)		not null,	/* 日期 */
	ModelCode	varchar(20)	not null,	/* 模型代码 */
	FactorCode	varchar(20)	not null,	/* 因子代码 */
	IndicatorCode	varchar(20)	not null,	/* 指标代码 */
	AvgAbstStat		numeric(20,4)	not null,	/* t值绝对值序列的平均值 */
	PercentAbstStat	numeric(20,4)	not null,	/* t值绝对值序列大于2的占比 */
	AvgIndicatorRet	numeric(20,4)	not null,	/* 指标收益率序列均值 */
	IndicatorRettStat	numeric(20,4)	not null,	/* 指标收益率序列的t值 */
	primary key (tmDate,ModelCode,FactorCode,IndicatorCode)
);

-- 因子检验数据
Create Table FactorTestData(
	tmDate		varchar(8)		not null,	/* 日期 */
	ModelCode	varchar(20)	not null,	/* 模型代码 */
	FactorCode	varchar(20)	not null,	/* 因子代码 */
	AvgAbstStat		numeric(20,4)	not null,	/* t值绝对值序列的平均值 */
	PercentAbstStat	numeric(20,4)	not null,	/* t值绝对值序列大于2的占比 */
	AvgFactorRet	numeric(20,4)	not null,	/* 指标收益率序列均值 */
	FactorRettStat	numeric(20,4)	not null,	/* 指标收益率序列的t值 */
	primary key (tmDate,ModelCode,FactorCode)
);

-- 证券预期收益
Create Table SecuExpectedReturn(
	tmDate		varchar(8)	not null,	/* 日期 */
	SecuCode	varchar(10)		not null,	/* 证券代码 */
	ModelCode	varchar(20)		not null,	/* 模型代码 */
	ExpectedReturn	numeric(20,4)	not null,	/* 预期收益率 */
	IndustryCode	varchar(50)	not null,	/* 行业分类代码 */
	primary key (tmDate,SecuCode,ModelCode)
);

-- 模型股票池
Create Table ModelSecuPool(
	tmDate		varchar(8)	not null,	/* 日期 */
	ModelCode	varchar(20)	not null,	/* 模型代码 */
	SecuCode	varchar(10)	not null,	/* 证券代码 */
	IndustryCode	varchar(50)	not null,	/* 行业代码 */
	Provider	varchar(20)	not null,	/* 行业分类提供商 */
	primary key (tmDate,ModelCode,SecuCode)
);

-- 模型投资组合
Create Table ModelPortfolio(
	tmDate		varchar(8)	not null,	/* 日期 */
	ModelCode	varchar(20)	not null,	/* 模型代码 */
	SecuCode	varchar(10)	not null,	/* 证券代码 */
	Weight		numeric(20,4)	not null,	/* 权重 */
	ExpectedReturn	numeric(20,4)	not null,	/* 预期收益率 */
	IndustryCode	varchar(50)	not null,	/* 行业代码 */
	Provider	varchar(20)	not null,	/* 行业分类提供商 */
	primary key (tmDate,ModelCode,SecuCode,Provider)
);

use StrategyData;
-- 策略基本信息
Create Table StrategyInfo(
	StrategyCode	varchar(50)		not null,	/* 策略代码 */
	Description		varchar(200)	not null,	/* 策略描述 */
	primary key (StrategyCode)
);

--------------------SmartMoneyQ策略相关表------------------------
-- SmartQ指标表
Create Table SmartQValue(
	tmDate 		varchar(8)		not null,	/* 日期(yyyymmdd) */
	SecuCode 	varchar(10)		not null,	/* 证券代码 */
	SmartQValue	numeric(20,4)	not null,	/* smartq值 */
	primary key (tmDate,SecuCode)
);