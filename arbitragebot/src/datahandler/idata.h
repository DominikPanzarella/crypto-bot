#pragma once
#include <string>
#include <chrono>

class IData {
public:
    virtual ~IData() = default;

    virtual std::string getSymbol() const = 0;

    virtual double getLastPrice() const = 0;
    
    virtual double getLastBid() const = 0;

    virtual double getLastAsk() const = 0;

    virtual double getOpenPrice() const = 0;

    virtual double getVolume() const = 0;

    virtual std::chrono::system_clock::time_point getTimestamp() const = 0;

    virtual double getMidPrice() const {
        return (getLastBid() + getLastAsk()) / 2.0;
    }
};