#pragma once

#include "datahandler/idata.h"

class TickerData : public IData
{
public:
    TickerData(std::string symbol,
               double lastPrice,
               double lastBid,
               double lastAsk,
               double openPrice,
               double volume,
               std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now());

    std::string getSymbol() const override;

    double getLastPrice() const override;

    double getLastBid() const override;

    double getLastAsk() const override;

    double getOpenPrice() const override;

    double getVolume() const override;

    std::chrono::system_clock::time_point getTimestamp() const override;

private:
    std::string m_symbol;

    double m_lastPrice = 0.0;

    double m_lastBid = 0.0;

    double m_lastAsk = 0.0;

    double m_openPrice = 0.0;

    double m_volume = 0.0;

    std::chrono::system_clock::time_point m_timestamp;
};