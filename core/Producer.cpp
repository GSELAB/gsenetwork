/*
 * Copyright (c) 2018 GSENetwork
 *
 * This file is part of GSENetwork.
 *
 * GSENetwork is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 */

#include <core/Producer.h>
#include <core/CommonIO.h>

namespace core {

Producer EmptyProducer;

Producer::Producer(Address const& address, int64_t timestamp): m_address(address), m_timestamp(timestamp)
{
    // TODO: CREATE A NEW PRODUCER
}

Producer::Producer(bytesConstRef data)
{
    unsigned index = 0;
    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() == PRODUCER_FIELDS) {
            m_address = rlp[index++].toHash<Address>(RLP::VeryStrict);
            m_timestamp = rlp[index++].toInt<int64_t>();
            m_votes = rlp[index++].toInt<uint64_t>();
        } else {
            BOOST_THROW_EXCEPTION(std::range_error("Producer RLP is not list or not PRODUCER_FIELDS!"));
        }
    } catch (Exception e) {
        //e << errinfo_name("Invalid producer format!");
        //throw;
        BOOST_THROW_EXCEPTION(e);
    }
}

Producer::~Producer()
{

}

Producer& Producer::operator=(Producer const& producer)
{
    if (this == &producer) return *this;
    m_address = producer.getAddress();
    m_timestamp = producer.getTimestamp();
    m_votes = producer.getVotes();
    for (auto const& i : producer.getVotersMap()) {
        m_votersMap.insert(i);
    }

    return *this;
}

bool Producer::operator==(Producer const& producer) const
{
    return (m_address == producer.getAddress()) && (m_timestamp == producer.getTimestamp()) &&
        (m_votes == producer.getVotes()) && (m_votersMap == producer.getVotersMap());
}

bool Producer::operator!=(Producer const& producer) const
{
    return !operator==(producer);
}

bool Producer::operator>(Producer const& producer) const
{
    if (m_votes > producer.getVotes()) {
        return true;
    } else if (m_votes == producer.getVotes()) {
        return m_address > producer.getAddress();
    } else {
        return false;
    }
}

bool Producer::operator<(Producer const& producer) const
{
    if (m_votes < producer.getVotes()) {
        return true;
    } else if (m_votes == producer.getVotes()) {
        return m_address < producer.getAddress();
    } else {
        return false;
    }
}

void Producer::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(PRODUCER_FIELDS);
    rlpStream << m_address
              << m_timestamp
              << m_votes;
}

void Producer::setVotes(uint64_t votes)
{
    m_votes = votes;
}

void Producer::addVoter(Address const& voter, uint64_t value)
{
    // TODO: ADD
}

bytes Producer::getKey()
{
    return m_address.asBytes();
}

bytes Producer::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

ProducerSnapshot::ProducerSnapshot(bytesConstRef data)
{
    populate(data);
}

ProducerSnapshot& ProducerSnapshot::operator=(ProducerSnapshot const& ps)
{
    if (this == &ps)
        return *this;

    m_timestamp = ps.getTimestamp();
    for (auto i : ps.getProducers())
        m_producers.push_back(i);
    return *this;
}

void ProducerSnapshot::populate(bytesConstRef data)
{
    m_producers.clear();
    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() > 0) {
            m_timestamp = rlp[0].toInt<int64_t>();
            for (unsigned i = 1; i < rlp.itemCount(); i++)
                m_producers.push_back(Producer(rlp[i].data()));
        }
    } catch (...) {

    }
}

void ProducerSnapshot::addProducer(Producer const& producer)
{
    if (!isExist(producer.getAddress()))
        m_producers.push_back(producer);
}

bool ProducerSnapshot::isExist(Address const& address) const
{
    for (auto i : m_producers)
        if (i.getAddress() == address)
            return true;
    return false;
}

void ProducerSnapshot::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(1 + m_producers.size());
    rlpStream << m_timestamp;
    for (auto i : m_producers)
        rlpStream.appendRaw(i.getRLPData());
}

bytes ProducerSnapshot::getKey()
{
    return toBytes(toString(m_timestamp));
}

bytes ProducerSnapshot::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

} // end of producer

