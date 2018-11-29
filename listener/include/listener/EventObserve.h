#pragma once

#include <vector>

#include <listener/Observer.h>

namespace listener {

template<typename ... Args>
class EventObserve {
public:
    EventObserve() {}

    virtual ~EventObserve() { m_observers.clear(); }

    void notify(Args ... args);

    void add(Observer<Args ...> const& observer) { m_observers.push_back(observer); }

    void remove(Observer<Args ...> const& observer) {
        auto itr = std::find(m_observers.begin(), m_observers.end(), observer);
        if (itr != m_observers.end()) {
            m_observers.erase(itr);
        }
    }

    std::vector<Observer<Args ...>> const& getObservers() const { return m_observers; }

private:
    std::vector<Observer<Args ...>> m_observers;
};

template<typename ... Args>
void EventObserve<Args ...>::notify(Args ... args) {
    for (auto& observer : m_observers) {
        observer.notify(std::forward<Args>(args)...);
    }
}
}