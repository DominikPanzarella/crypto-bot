#pragma once
#include <memory>

#include "datahandler/idata.h"

class IObserver{
public:
    virtual ~IObserver() = default;
    virtual void onData(std::shared_ptr<IData> data) = 0;

};