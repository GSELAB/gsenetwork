#pragma once

#include <vector>

#include <core/Address.h>
#include <core/Guards.h>
#include <core/Producer.h>

using namespace core;

namespace producer {

class ProducerScheduleType {
public:
    ProducerScheduleType() {}

    ProducerScheduleType& operator=(ProducerScheduleType const& pst);

    size_t size() const { return m_producers.size(); }

    bool isExist(Address const& address);

public:
    std::vector<Address> m_producers;
};

class Schedule {
public:
    Schedule();

    ~Schedule();

    std::vector<Producer> const& getActiceProducers() const { Guard l(x_activeProducers); return m_activeProducers; }

    void schedule();

    void addActiveProducer(Producer const& producer);

    void addActiveProducer(Address const& address);

    void removeActiveProducer(Producer const& producer);

    void removeActiveProducer(Address const& producer);

private:
    mutable Mutex x_activeProducers;
    std::vector<Producer> m_activeProducers;   //
};

}