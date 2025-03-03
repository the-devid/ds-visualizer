#include "gtest/gtest.h"

#include "src/observer.h"

#include <memory>
#include <sstream>
#include <stdexcept>

namespace NVis {

TEST(ObserverCorrectness, OneToOne) {
    std::stringstream out;
    Observable<int> actor([]() { return 38; });
    Observer<int> beholder([&out](int x) { out << x << "+"; }, [&out](int x) { out << x; }, [&out]() { out << "-"; });
    actor.Subscribe(&beholder);
    EXPECT_TRUE(beholder.IsSubscribed());
    actor.Notify(1);
    actor.Notify(5);
    EXPECT_EQ(out.str(), "38+15");
    actor.Notify(3);
    beholder.Unsubscribe();
    EXPECT_FALSE(beholder.IsSubscribed());
    EXPECT_EQ(out.str(), "38+153-");
}

TEST(ObserverCorrectness, OneToMany) {
    constexpr ssize_t kObserverCount = 10;
    std::vector<std::stringstream> outs(kObserverCount);
    Observable<int> actor([]() { return 0; });
    std::vector<std::unique_ptr<Observer<int>>> beholders;
    for (ssize_t i = 0; i < kObserverCount; ++i) {
        beholders.emplace_back(std::make_unique<Observer<int>>([&out = outs[i]]([[maybe_unused]] int x) { out << "+"; },
                                                               [&out = outs[i]](int x) { out << x; },
                                                               [&out = outs[i]]() { out << "-"; }));
        actor.Subscribe(beholders[i].get());
    }
    actor.Notify(8);
    actor.Notify(800);
    actor.Notify(555);
    actor.Notify(3535);
    for (ssize_t i = 0; i < kObserverCount; ++i) {
        EXPECT_EQ(outs[i].str(), "+88005553535");
    }
    beholders.clear();
    for (ssize_t i = 0; i < kObserverCount; ++i) {
        EXPECT_EQ(outs[i].str(), "+88005553535-");
    }
}

TEST(ObserverCorrectness, DoubleSubscribe) {
    std::stringstream out;
    auto observable = std::make_unique<Observable<int>>([]() { return 0; });
    auto observer = std::make_unique<Observer<int>>([&out]([[maybe_unused]] int x) { out << "+"; },
                                                    [&out](int x) { out << x; }, [&out]() { out << "-"; });
    observable->Subscribe(observer.get());
    observable->Subscribe(observer.get());
    observable->Notify(1);
    observer.reset();
    observable.reset();
    EXPECT_EQ(out.str(), "+-+1-");
}

TEST(ObserverCorrectness, DoubleUnsubscribe) {
    std::stringstream out;
    auto observable = std::make_unique<Observable<int>>([]() { return 0; });
    auto observer = std::make_unique<Observer<int>>([&out]([[maybe_unused]] int x) { out << "+"; },
                                                    [&out](int x) { out << x; }, [&out]() { out << "-"; });
    observable->Subscribe(observer.get());
    observable->Notify(1);
    observer->Unsubscribe();
    EXPECT_NO_FATAL_FAILURE(observer->Unsubscribe());
    EXPECT_EQ(out.str(), "+1-");
}

TEST(ObserverCorrectness, EraseObservableFirst) {
    std::stringstream out;
    auto observable = std::make_unique<Observable<int>>([]() { return 0; });
    auto observer = std::make_unique<Observer<int>>([&out]([[maybe_unused]] int x) { out << "+"; },
                                                    [&out](int x) { out << x; }, [&out]() { out << "-"; });
    observable->Subscribe(observer.get());
    observable->Notify(15);
    observable.reset();
    EXPECT_EQ(out.str(), "+15-");
    observer.reset();
    EXPECT_EQ(out.str(), "+15-");
}

TEST(ObserverCorrectness, EraseObsevrerFirst) {
    std::stringstream out;
    auto observable = std::make_unique<Observable<int>>([]() { return 0; });
    auto observer = std::make_unique<Observer<int>>([&out]([[maybe_unused]] int x) { out << "+"; },
                                                    [&out](int x) { out << x; }, [&out]() { out << "-"; });
    observable->Subscribe(observer.get());
    observable->Notify(15);
    observer.reset();
    EXPECT_EQ(out.str(), "+15-");
    observable.reset();
    EXPECT_EQ(out.str(), "+15-");
}

TEST(ObserverCorrectness, MultipleUnsubscribes) {
    std::stringstream out;
    auto observable = std::make_unique<Observable<int>>([]() { return 0; });
    auto observer = std::make_unique<Observer<int>>([&out]([[maybe_unused]] int x) { out << "+"; },
                                                    [&out](int x) { out << x; }, [&out]() { out << "-"; });
    observable->Subscribe(observer.get());
    observable->Notify(1);
    observer->Unsubscribe();
    observable->Notify(2);
    observable->Subscribe(observer.get());
    observable->Notify(3);
    observer->Unsubscribe();
    EXPECT_EQ(out.str(), "+1-+3-");
}

TEST(ObserverCorrectness, Exception) {
    std::stringstream out;
    auto observer = std::make_unique<Observer<int>>([&out]([[maybe_unused]] int x) { out << "+"; },
                                                    [&out](int x) { out << x; }, [&out]() { out << "-"; });
    try {
        auto observable = std::make_unique<Observable<int>>([]() { return 0; });
        observable->Subscribe(observer.get());
        observable->Notify(1);
        throw std::runtime_error{""};
    } catch (...) {
    }
    EXPECT_EQ(out.str(), "+1-");
}

} // namespace NVis
