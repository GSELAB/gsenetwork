#include <producer/Schedule.h>
#include <core/Log.h>
#include <config/Constant.h>

namespace producer {

ProducerScheduleType::ProducerScheduleType(bytesConstRef data)
{
    populate(data);
}

ProducerScheduleType& ProducerScheduleType::operator=(ProducerScheduleType const& pst)
{
    if (&pst == this) return *this;
    for (auto i : pst.m_producers)
        m_producers.push_back(i);
    return *this;
}

bool ProducerScheduleType::isExist(Address const& address)
{
    auto itr = std::find(m_producers.begin(), m_producers.end(), address);
    if (itr != m_producers.end()) return true;
    return false;
}

void ProducerScheduleType::populate(bytesConstRef data)
{
    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() > 0) {
            for (unsigned i = 0; i < rlp.itemCount(); i ++) {
                Address producer = rlp[i].toHash<Address>(RLP::VeryStrict);
                m_producers.push_back(producer);
            }
        }
    } catch (...) {

    }
}

void ProducerScheduleType::streamRLP(RLPStream& rlpStream) const
{
    if (!m_producers.empty()) {
        rlpStream.appendList(m_producers.size());
        for (auto i : m_producers)
            rlpStream << i;
    }
}

bytes ProducerScheduleType::getKey()
{
    return bytes();
}

bytes ProducerScheduleType::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

// ------------------------
Schedule::Schedule()
{

}

Schedule::~Schedule()
{

}

void Schedule::schedule()
{

}

void Schedule::addActiveProducer(Producer const& producer)
{

}

void Schedule::addActiveProducer(Address const& address)
{

}

void Schedule::removeActiveProducer(Producer const& producer)
{

}

void Schedule::removeActiveProducer(Address const& producer)
{

}

void Schedule::addProducer(Producer const& producer)
{
    //Guard l{x_producerList};
    //m_producerList.push_back(producer);
}

Address Schedule::getAddress(unsigned idx) const
{
    Guard l{x_currentProducerList};
    if (idx >= m_currentProducerList.size())
        return ZeroAddress;
    return m_currentProducerList[idx].getAddress();
}

void Schedule::producerSort()
{
    {
        Guard l{x_prevProducerList};
        std::sort(m_prevProducerList.begin(), m_prevProducerList.end(), ProducerCompareLess());
    }

    {
        Guard l{x_currentProducerList};
        std::sort(m_currentProducerList.begin(), m_currentProducerList.end(), ProducerCompareLess());
    }
}

Producers Schedule::getCurrentProducerList() const
{
    Guard l{x_currentProducerList};
    Producers ret;
    int count = 0;
    for (auto i: m_currentProducerList) {
        if (count < NUM_DELEGATED_BLOCKS) {
            ret.push_back(i);
            count++;
        }
    }

    return ret;

}

void Schedule::schedule(ProducersConstRef producerList)
{
    {
        Guard l(x_prevProducerList);
        if (!m_prevProducerList.empty())
            m_prevProducerList.clear();
    }
    {
        Guard l{x_currentProducerList};
        if (!m_currentProducerList.empty()) {
            for (auto i : m_currentProducerList)
                    m_prevProducerList.push_back(i);
            m_currentProducerList.clear();
        }

        for (auto i : producerList)
            m_currentProducerList.push_back(i);
    }
    producerSort();
}

} // namespace end
