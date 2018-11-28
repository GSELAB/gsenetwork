#pragma once

#include <vector>

#include <listener/Observer.h>

namespace listener {

template<typename ... Args>
class EventObserve {
public:
    EventObserve() = default;

    EventObserve(EventObserve const& observe);

    EventObserve& operator=(EventObserve const& observe);

    virtual ~EventObserve() { m_observers.clear(); }

    void notify(Args ... arg);

    bool add(std::function<Args ...> observer) { m_observers.push_back(observer); }

    void remove(std::function<Args ...> observer) {
        auto itr = std::find(m_observers.begin(), m_observers.end(), observer);
        if (itr != m_observers.end()) {
            m_observers.erase(itr);
        }
    }

    std::vector<Observer<Args ...>> const& getObservers() const { return m_observers; }

private:
    std::vector<Observer<Args ...>> m_observers;
};
}