#pragma once

#include "observer/isubject.h"
#include "observer/iobserver.h"

#include <shared_mutex>
#include <unordered_set>
#include <memory>

class WebSocketSubject : public ISubject
{
public:

    WebSocketSubject() = default;

    virtual void subscribe(std::shared_ptr<IObserver> observer) override;

    virtual void unsubscribe(std::shared_ptr<IObserver> observer) override;

    virtual void notify(std::shared_ptr<IData> data) override;

private:
    std::unordered_set<std::shared_ptr<IObserver>> observers_;
    mutable std::shared_mutex mutex_;
};