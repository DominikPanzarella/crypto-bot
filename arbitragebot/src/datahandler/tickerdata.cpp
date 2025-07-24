#include "datahandler/tickerdata.h"

TickerData::TickerData(std::string symbol,
                       double lastPrice,
                       double lastBid,
                       double lastAsk,
                       double openPrice,
                       double volume,
                       std::chrono::system_clock::time_point timestamp)
    : m_symbol(std::move(symbol)),
      m_lastPrice(lastPrice),
      m_lastBid(lastBid),
      m_lastAsk(lastAsk),
      m_openPrice(openPrice),
      m_volume(volume),
      m_timestamp(timestamp)
{}

std::string TickerData::getSymbol() const {
    return m_symbol;
}

double TickerData::getLastPrice() const {
    return m_lastPrice;
}

double TickerData::getLastBid() const {
    return m_lastBid;
}

double TickerData::getLastAsk() const {
    return m_lastAsk;
}

double TickerData::getOpenPrice() const {
    return m_openPrice;
}

double TickerData::getVolume() const {
    return m_volume;
}

std::chrono::system_clock::time_point TickerData::getTimestamp() const {
    return m_timestamp;
}
