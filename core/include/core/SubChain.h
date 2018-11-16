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

#include <core/Object.h>
#include <chain/Types.h>
#include <core/Address.h>
#include <core/Common.h>
#include <core/RLP.h>
#include <core/FixedHash.h>

namespace core {

#define SUB_CHAIN_FIELDS 3

class SubChain: public Object {
public:
    SubChain();

    SubChain(bytesConstRef data);

    ~SubChain();

    void streamRLP(RLPStream& rlpStream) const;

    bool operator==(SubChain const& subChain);

    void setChainID(chain::ChainID chainID);

    void setCreator(Address const& address);

    void setTimestamp(int64_t timestamp);

    chain::ChainID getChainID() const;

    Address const& getCreator() const;

    int64_t getTimestamp() const;

    // @override
    bytes getKey();

    // @override
    bytes getRLPData();

    // @override
    Object::ObjectType getObjectType() const { return Object::SubChainType; }

private:
    chain::ChainID m_chainID;
    Address m_creator;
    int64_t m_timestamp;

    h256 m_hash;
};

} // end namespace