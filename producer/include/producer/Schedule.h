#pragma once

#include <vector>

#include <core/Address.h>
#include <core/Guards.h>
#include <core/Producer.h>
#include <core/Object.h>

using namespace core;

namespace producer {

class ProducerScheduleType: public core::Object {
public:
    ProducerScheduleType() = default;

    ProducerScheduleType(bytesConstRef data);

    ProducerScheduleType& operator=(ProducerScheduleType const& pst);

    size_t size() const { return m_producers.size(); }

    bool isExist(Address const& address);

    void populate(bytesConstRef data);

    void streamRLP(core::RLPStream& rlpStream) const;

    virtual bytes getKey() override;

    virtual bytes getRLPData() override;

    virtual Object::ObjectType getObjectType() const override { return Object::ProducerScheduleTypeType; }

public:
    std::vector<Address> m_producers;
};

// down
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

    /// @not thread safe
    ProducersConstRef getActiveProducers() const { return m_activeProducers; }

    void schedule();

    void addActiveProducer(Producer const& producer);

    void addActiveProducer(Address const& address);

    void removeActiveProducer(Producer const& producer);

    void removeActiveProducer(Address const& producer);

public: // used by producer or block chain
    void addProducer(Producer const& producer);

    void producerSort();

    ProducersConstRef getProducerList() const { return m_currentProducerList; }

    Producers getCurrentProducerList() const;

    Address getAddress(unsigned idx) const;

    void schedule(ProducersConstRef producerList);

private:
    mutable Mutex x_activeProducers;
    Producers m_activeProducers;

    mutable Mutex x_prevProducerList;
    std::vector<Producer> m_prevProducerList;

    mutable Mutex x_currentProducerList;
    std::vector<Producer> m_currentProducerList;
};

}
