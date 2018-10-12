#pragma once

#include <vector>

#include <core/Address.h>

using namespace core;

namespace producer {

class ProducerScheduleType {
public:
    ProducerScheduleType() {}

    ProducerScheduleType& operator=(ProducerScheduleType const& pst);

    size_t size() const { return m_producers.size(); }

public:
    std::vector<Address> m_producers;
};

}