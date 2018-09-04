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

Address const& Producer::getAddress() const
{
    return m_address;
}

int64_t Producer::getTimestamp() const
{
    return m_timestamp;
}

uint64_t Producer::getVotes() const
{
    return m_votes;
}

// @override
std::string Producer::getKey()
{
    return m_address.ref().toString();
}

// @override
std::string Producer::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return bytesConstRef(&rlpStream.out()).toString();
}

} // end of producer