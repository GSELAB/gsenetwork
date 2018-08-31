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

#include <core/Block.h>
#include <core/Exceptions.h>
#include <crypto/SHA3.h>

using namespace core;
using namespace crypto;

BlockHeader::BlockHeader()
{
    // TODO: DO NOTHING
}

BlockHeader::BlockHeader(bytes const& data): BlockHeader(&data)
{
    // TODO: DO NOTHING
}

BlockHeader::BlockHeader(bytes const& data, h256 const& hash)
{
    BlockHeader(&data, hash);
}

BlockHeader::BlockHeader(bytesConstRef data)
{
    RLP rlpHeader = RLP(data);
    m_hash = sha3(rlpHeader.data());
    populate(rlpHeader);
}

BlockHeader::BlockHeader(bytesConstRef data, h256 const& hash)
{
    RLP rlpHeader = RLP(data);
    m_hash = hash ? hash : sha3(rlpHeader.data());
    populate(rlpHeader);
}

BlockHeader::BlockHeader(BlockHeader const& header)
{
    m_producer = header.getProducer();
    m_parentHash = header.getParentHash();
    m_number = header.getNumber();
    m_timestamp = header.getTimestamp();
    m_extra = header.getExtra();
    m_hash = header.getHash(); // sha3 of the blockheader
}

BlockHeader& BlockHeader::operator=(BlockHeader const& header)
{
    if (&header  == this) {
        return *this;
    }

    m_producer = header.getProducer();
    m_parentHash = header.getParentHash();
    m_number = header.getNumber();
    m_timestamp = header.getTimestamp();
    m_extra = header.getExtra();
    m_hash = header.getHash();

    return *this;
}

bool BlockHeader::operator==(BlockHeader const& header) const
{
    return  (m_producer == header.getProducer()) && (m_parentHash == header.getParentHash()) &&
        (m_number == header.getNumber()) && (m_timestamp == header.getTimestamp()) &&
        (m_extra == header.getExtra());
}

bool BlockHeader::operator!=(BlockHeader const& header) const
{
    return !operator==(header);
}

void BlockHeader::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(BLOCK_HEADER_FIELDS);
    rlpStream << m_producer
              << m_parentHash
              <<m_mklRoot
              << m_transactionsRoot
              << m_receiptRoot
              << m_number
              << m_timestamp
              << m_extra;
}

void BlockHeader::populate(RLP const& rlp)
{
    unsigned index = 0;
    try {
        m_producer = rlp[index = 0].toHash<Address>(RLP::VeryStrict);
        m_parentHash = rlp[index = 1].toHash<h256>(RLP::VeryStrict);
        m_mklRoot = rlp[index = 2].toHash<h256>(RLP::VeryStrict);
        m_transactionsRoot = rlp[index = 3].toHash<h256>(RLP::VeryStrict);
        m_receiptRoot = rlp[index = 4].toHash<h256>(RLP::VeryStrict);
        m_number = rlp[index = 5].toPositiveInt64();
        m_timestamp = rlp[index = 6].toPositiveInt64();
        m_extra = rlp[index = 7].toBytes();
    } catch (Exception const& e) {
        e << errinfo_name("Interpret RLP header failed") << BadFieldError(index, toHex(rlp[index].data().toBytes()));
        throw;
    }
}

void BlockHeader::populateFromParent(BlockHeader const& header)
{
    // TODO: DO NOTHING
}

void BlockHeader::setProducer(Address const& producer)
{
    m_producer = producer;
}

void BlockHeader::setParentHash(h256 const& parentHash)
{
    m_parentHash = parentHash;
}

void BlockHeader::setNumber(uint64_t number)
{
    m_number = number;
}

void BlockHeader::setTimestamp(int64_t timestamp)
{
    m_timestamp = timestamp;
}

void BlockHeader::setExtra(bytes const& extra)
{
    m_extra = extra;
}

void BlockHeader::setHash(h256 const& hash)
{
    m_hash = hash;
}

Address const& BlockHeader::getProducer() const
{
    return m_producer;
}

h256 const& BlockHeader::getParentHash() const
{
    return m_parentHash;
}

uint64_t BlockHeader::getNumber() const
{
    return m_number;
}

int64_t BlockHeader::getTimestamp() const
{
    return m_timestamp;
}

bytes const& BlockHeader::getExtra() const
{
    return m_extra;
}

h256 const& BlockHeader::getHash() const
{
    return m_hash;
}

void BlockHeader::clear()
{
    m_producer = Address();
    m_parentHash = h256();
    m_number = 0;
    m_timestamp = -1;
    m_extra.clear();
    m_hash = h256();
}