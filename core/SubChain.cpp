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

#include <core/SubChain.h>
#include <core/Exceptions.h>
#include <crypto/SHA3.h>

using namespace crypto;
using namespace core;

SubChain::SubChain()
{
    // TODO: DO NOTHING
}

SubChain::SubChain(bytesConstRef data)
{
    unsigned index = 0;
    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() == SUB_CHAIN_FIELDS) {
            m_chainID = rlp[index++].toInt<chain::ChainID>();
            m_creator = rlp[index++].toHash<Address>(RLP::VeryStrict);
            m_timestamp = rlp[index++].toInt<int64_t>();
        } else {
            BOOST_THROW_EXCEPTION(std::range_error("SubChain RLP must be a list"));
        }
    } catch (Exception e) {
        e << errinfo_name("Invalid Sub Chain format");
        throw;
    }
}

SubChain::~SubChain()
{

}

void SubChain::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(SUB_CHAIN_FIELDS);
    rlpStream << (bigint) m_chainID
              << m_creator
              << (bigint) m_timestamp;
}

bool SubChain::operator==(SubChain const& subChain)
{
    return (m_chainID == subChain.getChainID()) &&
        (m_creator == subChain.getCreator()) &&
        (m_timestamp == subChain.getTimestamp());
}

void SubChain::setChainID(chain::ChainID chainID)
{
    m_chainID = chainID;
}

void SubChain::setCreator(Address const& address)
{
    m_creator = address;
}

void SubChain::setTimestamp(int64_t timestamp)
{
    m_timestamp = timestamp;
}

chain::ChainID SubChain::getChainID() const
{
    return m_chainID;
}

Address const& SubChain::getCreator() const
{
    return m_creator;
}

int64_t SubChain::getTimestamp() const
{
    return m_timestamp;
}

// @override
bytes SubChain::getKey()
{
    if (m_hash) {
        return m_hash.asBytes();
    }


    RLPStream rlpStream;
    streamRLP(rlpStream);
    m_hash = sha3(&rlpStream.out());
    return m_hash.asBytes();
}

// @override
bytes SubChain::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}
