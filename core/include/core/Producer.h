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

#include<vector>

#include <core/Address.h>
#include <core/Object.h>
#include <core/RLP.h>

namespace core {

#define PRODUCER_FIELDS (3)

class Producer: public Object {
public:
    Producer() {}

    Producer(Address const& address, int64_t timestamp);

    Producer(bytesConstRef data);

    ~Producer();

    Producer& operator=(Producer const& producer);

    bool operator==(Producer const& producer) const;

    bool operator!=(Producer const& producer) const;

    bool operator>(Producer const& producer) const;

    bool operator<(Producer const& producer) const;

    void streamRLP(RLPStream& rlpStream) const;

    void setVotes(uint64_t votes);

    void addVoter(Address const& voter, uint64_t value);

    Address const& getAddress() const { return m_address; }

    int64_t getTimestamp() const { return m_timestamp; }

    std::map<Address, uint64_t> const& getVotersMap() const { return m_votersMap; }

    uint64_t getVotes() const { return m_votes; }

    // @override
    bytes getKey();

    // @override
    bytes getRLPData();

    // @override
    Object::ObjectType getObjectType() const { return Object::BlockType; }

private:
    Address m_address;
    int64_t m_timestamp;
    std::map<Address, uint64_t> m_votersMap;
    uint64_t m_votes;
};

extern Producer EmptyProducer;

using ProducerPtr = std::shared_ptr<Producer>;

using Producers = std::vector<Producer>;

using ProducersRef = Producers&;

using ProducersConstRef = Producers const&;

} // end of namespace