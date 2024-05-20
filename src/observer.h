#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <list>
#include <utility>

namespace NVis {

template <typename TData>
class Observable;

template <typename TData>
class Observer {
    friend Observable<TData>;

public:
    template <typename TSub, typename TNotify, typename TUnsub>
    Observer(TSub&& on_subscribe, TNotify&& on_notify, TUnsub&& on_unsubscribe)
        : on_subscribe_(std::forward<TSub>(on_subscribe)),
          on_notify_(std::forward<TNotify>(on_notify)),
          on_unsubscribe_(std::forward<TUnsub>(on_unsubscribe)) {}

    Observer(const Observer&) = delete;
    Observer& operator=(const Observer&) = delete;
    Observer(Observer&&) = delete;
    Observer& operator=(Observer&&) = delete;

    ~Observer() {
        Unsubscribe();
    }

    void Unsubscribe() {
        if (IsSubscribed()) {
            observable_->Detach(this);
            observable_ = nullptr;
        }
    }
    bool IsSubscribed() const {
        return observable_ != nullptr;
    }

private:
    void SetObservable(Observable<TData>* observable) {
        observable_ = observable;
    }
    Observable<TData>* observable_ = nullptr;

    std::function<void(const TData&)> on_subscribe_;
    std::function<void(const TData&)> on_notify_;
    std::function<void()> on_unsubscribe_;
};

template <typename TData>
class Observable {
public:
    template <typename TSubscribeDataFunc>
    Observable(TSubscribeDataFunc&& subscribe_data_func)
        : subscribe_data_(std::forward<TSubscribeDataFunc>(subscribe_data_func)) {}

    Observable() = delete;
    Observable(const Observable&) = delete;
    Observable& operator=(const Observable&) = delete;
    Observable(Observable&&) = delete;
    Observable& operator=(Observable&&) = delete;

    ~Observable() {
        while (!subscribers_.empty()) {
            subscribers_.front()->Unsubscribe();
        }
    }

    void Subscribe(Observer<TData>* observer) {
        assert(observer != nullptr && "Trying to subscribe non-existing observer");
        if (observer->IsSubscribed()) {
            observer->Unsubscribe();
        }
        subscribers_.emplace_back(observer);
        observer->SetObservable(this);
        observer->on_subscribe_(subscribe_data_());
    }
    void Notify(TData data) const {
        for (auto subscriber : subscribers_) {
            subscriber->on_notify_(data);
        }
    }

private:
    void Detach(Observer<TData>* observer) {
        observer->on_unsubscribe_();
        subscribers_.remove(observer);
    }
    std::list<Observer<TData>*> subscribers_;
    std::function<TData()> subscribe_data_;
    friend Observer<TData>;
};

} // namespace NVis
