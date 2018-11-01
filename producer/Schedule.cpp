#include <producer/Schedule.h>

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

//@not thread safe
std::vector<Producer> const& Schedule::getActiveProducers() const
{
    return m_activeProducers;
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

std::vector<Producer> const & Schedule::getProducerList() const {
    return m_producerList;
}

void Schedule::addProducer(Producer const& producer)
{
    Guard l{x_producerList};
    m_producerList.push_back(producer);
}

void Schedule::producerSort()
{
    Guard l{x_producerList};
    std::sort(m_producerList.begin(), m_producerList.end(), ProducerCompareGreater());
}

}
