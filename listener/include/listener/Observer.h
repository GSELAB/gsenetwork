#pragma once

#include <functional>

namespace listener {

template<typename ... Args>
class Observer {
public:
    Observer(std::function<void(Args ...)> handler): m_handler(handler) {}

    Observer(Observer const& observer) {
        m_handler = observer.getHandler();
    }

    Observer& operator=(Observer const& observer) {
        if (&observer == this) {
            return *this;
        }

        m_handler = observer.getHandler();
        return *this;
    }

    void notify(Args ... args) {
        if (m_handler) {
            m_handler(args ...);
        }
    }

    void setHandler(std::function<void(Args ...)> const& handler) { m_handler = handler; }

    std::function<void(Args ...)> const& getHandler() const { return m_handler; }

private:
    std::function<void(Args ...)> m_handler = nullptr;
};
}