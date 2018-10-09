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

// @override
bytes Producer::getKey()
{
    return m_address.asBytes();
}

// @override
bytes Producer::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

} // end of producer