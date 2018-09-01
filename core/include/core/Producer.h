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
    Producer(Address const& address, int64_t timestamp);

    Producer(bytesConstRef data);

    ~Producer();

    void streamRLP(RLPStream& rlpStream) const;

    void setVotes(uint64_t votes);

    Address const& getAddress() const;

    int64_t getTimestamp() const;

    uint64_t getVotes() const;

    // @override
    std::string getKey();

    // @override
    std::string getRLPData();

private:
    Address m_address;
    int64_t m_timestamp;
    uint64_t m_votes;
};

} // end of namespace