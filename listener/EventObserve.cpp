#include <listener/EventObserve.h>

namespace listener {

template<typename ... Args>
EventObserve<Args ...>::EventObserve(EventObserve const& observe)
{
    for (auto& i : observe.getObservers()) {
        m_observers.push_back(i);
    }
}

template<typename ... Args>
EventObserve<Args ...>& EventObserve<Args ...>::operator=(EventObserve const& observe)
{
    m_observers.clear();
    for (auto i : observe.getListeners()) {
        m_observers.push_back(i);
    }
}

template<typename ... Args>
void EventObserve<Args ...>::notify(Args ... args)
{
    for (auto& observer : m_observers) {
        observer.notify(args ...);
    }
}
}