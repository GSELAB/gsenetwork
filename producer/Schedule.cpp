#include <producer/Schedule.h>

namespace producer {

ProducerScheduleType& ProducerScheduleType::operator=(ProducerScheduleType const& pst)
{
    if (&pst == this) return *this;
    for (auto i : pst.m_producers)
        m_producers.push_back(i);
    return *this;
}

}