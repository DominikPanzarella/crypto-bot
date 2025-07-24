#include "observer/websocketsubject.h"

void WebSocketSubject::subscribe(std::shared_ptr<IObserver> observer) {
    std::unique_lock lock(mutex_);
    observers_.insert(observer);
}

void WebSocketSubject::unsubscribe(std::shared_ptr<IObserver> observer) {
    std::unique_lock lock(mutex_);
    observers_.erase(observer);
}

void WebSocketSubject::notify(std::shared_ptr<IData> data) {
    std::shared_lock lock(mutex_);
    for (const auto& obs : observers_) {
        if (obs) obs->onData(data);
    }
}