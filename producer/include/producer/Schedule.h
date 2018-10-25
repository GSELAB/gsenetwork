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

// Up
class ProducerCompareLess {
public:
    bool operator()(Producer const& first, Producer const& second) { return first > second; }
};

class ProducerCompareGreater {
public:
    bool operator()(Producer const& first, Producer const& second) { return first < second; }
};

class Schedule {
public:
    Schedule();

    ~Schedule();

    std::vector<Producer> const& getActiveProducers() const;

    void schedule();

    void addActiveProducer(Producer const& producer);

    void addActiveProducer(Address const& address);

    void removeActiveProducer(Producer const& producer);

    void removeActiveProducer(Address const& producer);

public: // used by producer or block chain
    void addProducer(Producer const& producer);

    void producerSort();

private:
    mutable Mutex x_activeProducers;
    std::vector<Producer> m_activeProducers;

    mutable Mutex x_producerList;
    std::vector<Producer> m_producerList;
};

}
