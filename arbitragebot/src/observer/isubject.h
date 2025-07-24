#pragma once

#include "observer/iobserver.h"

class ISubject{
public:
    virtual ~ISubject() = default;

    virtual void subscribe(std::shared_ptr<IObserver> observer) = 0;

    virtual void unsubscribe(std::shared_ptr<IObserver> observer) = 0;
    
    virtual void notify(std::shared_ptr<IData> data) = 0;
};
