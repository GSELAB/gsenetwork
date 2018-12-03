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

Producer::Producer(Producer const& producer)
{
    m_address = producer.getAddress();
    m_timestamp = producer.getTimestamp();
    m_votes = producer.getVotes();
    m_voters.clear();
    for (auto& i : producer.getVoters()) {
        m_voters[i.first] = i.second;
    }
}

Producer::Producer(bytesConstRef data)
{
    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() == PRODUCER_FIELDS) {
            m_address = rlp[0].toHash<Address>(RLP::VeryStrict);
            m_timestamp = rlp[1].toInt<int64_t>();
            m_votes = rlp[2].toInt<uint64_t>();
            bytesConstRef vBytes = rlp[3].data();
            RLP rlpVoters(vBytes);
            if (rlpVoters.isList() && rlpVoters.itemCount() > 0) {
                for (unsigned i = 0; i < rlpVoters.itemCount(); ) {
                    Address address = rlpVoters[i++].toHash<Address>(RLP::VeryStrict);
                    uint64_t vote = rlpVoters[i++].toInt<uint64_t>();
                    m_voters[address] = vote;
                }
            }
        } else {
            BOOST_THROW_EXCEPTION(std::range_error("Producer RLP is not list or not PRODUCER_FIELDS!"));
        }
    } catch (Exception e) {
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
    m_voters.clear();
    for (auto& i : producer.getVoters()) {
        m_voters[i.first] = i.second;
    }

    return *this;
}

bool Producer::operator==(Producer const& producer) const
{
    return (m_address == producer.getAddress()) && (m_timestamp == producer.getTimestamp());
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
              << (bigint) m_timestamp
              << (bigint) m_votes;
    RLPStream votersRLPStream;
    votersRLPStream.appendList(m_voters.size() * 2);
    for (auto i : m_voters)
        votersRLPStream << i.first << (bigint)i.second;
    rlpStream.appendRaw(votersRLPStream.out());
}

void Producer::setVotes(uint64_t votes)
{
    m_votes = votes;
}

void Producer::addVoter(Address const& voter, uint64_t value)
{
    m_voters[voter] = value;
    m_votes += value;
}

void Producer::eraseVoter(Address const& voter)
{
    auto itr = m_voters.find(voter);
    if (itr != m_voters.end()) {
        m_votes -= itr->second;
        m_voters.erase(itr);
    }
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

ProducerSnapshot::ProducerSnapshot(ProducerSnapshot const& ps)
{
    m_producers.clear();
    m_timestamp = ps.getTimestamp();
    for (auto i : ps.getProducers())
        m_producers.push_back(i);
}

ProducerSnapshot::ProducerSnapshot(bytesConstRef data)
{
    populate(data);
}

ProducerSnapshot& ProducerSnapshot::operator=(ProducerSnapshot const& ps)
{
    if (this == &ps)
        return *this;

    m_producers.clear();
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

void ProducerSnapshot::deleteProducer(Producer const& producer)
{
    Producers::iterator itr;
    itr = std::find(m_producers.begin(), m_producers.end(), producer);
    if (itr != m_producers.end())
        m_producers.erase(itr);
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
    rlpStream << (bigint) m_timestamp;
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

ActiveProducerSnapshot::ActiveProducerSnapshot(ActiveProducerSnapshot const& ps)
{
    m_producers.clear();
    m_timestamp = ps.getTimestamp();
    for (auto i : ps.getProducers())
        m_producers.push_back(i);
}

ActiveProducerSnapshot::ActiveProducerSnapshot(bytesConstRef data)
{
    populate(data);
}

ActiveProducerSnapshot& ActiveProducerSnapshot::operator=(ActiveProducerSnapshot const& ps)
{
    if (this == &ps)
        return *this;

    m_producers.clear();
    m_timestamp = ps.getTimestamp();
    for (auto i : ps.getProducers())
        m_producers.push_back(i);
    return *this;
}

void ActiveProducerSnapshot::populate(bytesConstRef data)
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

void ActiveProducerSnapshot::addProducer(Producer const& producer)
{
    if (!isExist(producer.getAddress()))
        m_producers.push_back(producer);
}

void ActiveProducerSnapshot::deleteProducer(Producer const& producer)
{
    Producers::iterator itr;
    itr = std::find(m_producers.begin(), m_producers.end(), producer);
    if (itr != m_producers.end())
        m_producers.erase(itr);
}

bool ActiveProducerSnapshot::isExist(Address const& address) const
{
    for (auto i : m_producers)
        if (i.getAddress() == address)
            return true;
    return false;
}

void ActiveProducerSnapshot::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(1 + m_producers.size());
    rlpStream << (bigint) m_timestamp;
    for (auto i : m_producers)
        rlpStream.appendRaw(i.getRLPData());
}

bytes ActiveProducerSnapshot::getKey()
{
    return toBytes(toString(m_timestamp));
}

bytes ActiveProducerSnapshot::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}
} // end of producer

