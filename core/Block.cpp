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

BlockHeader::BlockHeader(uint64_t number): m_number(number)
{

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
    rlpStream << m_chainID
              << m_producer
              << m_parentHash
              << m_mklRoot
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
        m_chainID = rlp[index++].toInt<chain::ChainID>();
        m_producer = rlp[index++].toHash<Address>(RLP::VeryStrict);
        m_parentHash = rlp[index++].toHash<h256>(RLP::VeryStrict);
        m_mklRoot = rlp[index++].toHash<h256>(RLP::VeryStrict);
        m_transactionsRoot = rlp[index++].toHash<h256>(RLP::VeryStrict);
        m_receiptRoot = rlp[index++].toHash<h256>(RLP::VeryStrict);
        m_number = rlp[index++].toPositiveInt64();
        m_timestamp = rlp[index++].toPositiveInt64();
        m_extra = rlp[index++].toBytes();
    } catch (Exception const& e) {
        e << errinfo_name("Interpret RLP header failed") << BadFieldError(index, toHex(rlp[index - 1].data().toBytes()));
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

// @override
std::string BlockHeader::getKey()
{
    if (m_hash) {
        return m_hash.ref().toString();
    }

    RLPStream rlpStream;
    streamRLP(rlpStream);
    m_hash = sha3(&rlpStream.out());
    return m_hash.ref().toString();
}

// @override
std::string BlockHeader::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return bytesConstRef(&rlpStream.out()).toString();
}

/* --- Block ---*/
Block::Block()
{

}

Block::Block(BlockHeader const&blockHeader)
{
    m_blockHeader = blockHeader;
}

Block::Block(Block const& block)
{

}

Block::Block(bytesConstRef data)
{
    try {
        RLP rlp(data);



    } catch (Exception e) {
        BOOST_THROW_EXCEPTION(e);
    }
}

void Block::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(3);

    {
        // block header rlp
        RLPStream rlpStreamBlockHeader;
        m_blockHeader.streamRLP(rlpStreamBlockHeader);

        rlpStream.appendRaw(rlpStreamBlockHeader.out());
    }

    {
        // transactions rlp
        RLPStream rlpStreamTransactions;
        rlpStreamTransactions.appendList(m_transactions.size());
        for (unsigned i = 0, m = m_transactions.size(); i < m; i++) {
            RLPStream rlpStreamTransaction;
            m_transactions[i].streamRLP(rlpStreamTransaction);
            rlpStreamTransactions.appendRaw(rlpStreamTransaction.out());
        }

        rlpStream.appendRaw(rlpStreamTransactions.out());
    }

    {
        RLPStream rlpStreamReceipts;
        rlpStreamReceipts.appendList(m_transactionReceipts.size());
        for (unsigned i = 0, m = m_transactionReceipts.size(); i < m; i++) {
            RLPStream rlpStreamReceipt;
            m_transactionReceipts[i].streamRLP(rlpStreamReceipt);
            rlpStreamReceipts.appendRaw(rlpStreamReceipt.out());
        }

        rlpStream.appendRaw(rlpStreamReceipts.out());
    }
}

Block& Block::operator=(Block const& block)
{
    // Incomplete compare
    m_blockHeader = block.getBlockHeader();

    // add transactions & receipts
    return *this;
}

bool Block::operator==(Block const& block) const
{
    // Incomplete compare
    return (m_blockHeader == block.getBlockHeader());
}

bool Block::operator!=(Block const& block) const
{
    return !operator==(block);
}

void Block::setBlockHeader(BlockHeader const& blockHeader)
{
    m_blockHeader = blockHeader;
}

void Block::addTransaction(Transaction const& transaction)
{
    // TODO:
}

void Block::addTransactionReceipt(TransactionReceipt const& transactionReceipt)
{
    // TODO:
}

Address Block::getProducer() const
{
    return m_blockHeader.getProducer();
}

BlockHeader const& Block::getBlockHeader() const
{
    return m_blockHeader;
}

Transactions const& Block::getTransactions() const
{
    return m_transactions;
}

TransactionReceipts const& Block::getTransactionReceipts() const
{
    return m_transactionReceipts;
}

// @override
std::string Block::getKey()
{
    if (m_hash) {
        return m_hash.ref().toString();
    }

    RLPStream rlpStream;
    streamRLP(rlpStream);
    m_hash = sha3(&rlpStream.out());
    return m_hash.ref().toString();
}

// @override
std::string Block::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return bytesConstRef(&rlpStream.out()).toString();
}