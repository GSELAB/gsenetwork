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

#pragma once

#include "core/Transaction.h"
#include "core/Types.h"
#include "core/Address.h"
#include <core/Common.h>
#include <core/RLP.h>

namespace core {

#define BLOCK_HEADER_FIELDS (8)

class BlockHeader {
public:
    BlockHeader();

    BlockHeader(bytes const& data);

    BlockHeader(bytes const& data, h256 const& hash);

    BlockHeader(bytesConstRef data);

    BlockHeader(bytesConstRef data, h256 const& hash);

    BlockHeader(BlockHeader const& header);

    BlockHeader& operator=(BlockHeader const& header);

    bool operator==(BlockHeader const& header) const;

    bool operator!=(BlockHeader const& header) const;

    void streamRLP(RLPStream& rlpStream) const;

    void populate(RLP const& rlp);

    void populateFromParent(BlockHeader const& header);

    void setProducer(Address const& producer);

    void setParentHash(h256 const& parentHash);

    void setNumber(uint64_t number);

    void setTimestamp(int64_t timestamp);

    void setExtra(bytes const& extra);

    void setHash(h256 const& hash);

    Address const& getProducer() const;

    h256 const& getParentHash() const;

    uint64_t getNumber() const;

    int64_t getTimestamp() const;

    bytes const& getExtra() const;

    h256 const& getHash() const;

    void clear();
private:
    Address  m_producer;
    h256 m_parentHash;
    h256 m_mklRoot;
    h256 m_transactionsRoot;
    h256 m_receiptRoot;
    uint64_t m_number;
    int64_t m_timestamp;
    bytes m_extra;

    h256 m_hash; // sha3 of the blockheader
};

class Block {
public:
    Block();

    Block(Block const& block);

    Block& operator=(Block const& block);

    bool operator==(Block const& block) const;

    bool operator!=(Block const& block) const;



private:
    BlockHeader m_blockHeader;
    Transactions m_transactions;


};

}  /* namespace end */