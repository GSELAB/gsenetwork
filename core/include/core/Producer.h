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

#define PRODUCER_FIELDS (4)

class Producer: public Object {
public:
    Producer() {}

    Producer(Producer const& producer);

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

    void eraseVoter(Address const& voter);

    Address const& getAddress() const { return m_address; }

    int64_t getTimestamp() const { return m_timestamp; }

    std::map<Address, uint64_t> const& getVoters() const { return m_voters; }

    uint64_t getVotes() const { return m_votes; }

    virtual bytes getKey() override;

    virtual bytes getRLPData() override;

    virtual Object::ObjectType getObjectType() const override { return Object::ProducerType; }

private:
    Address m_address;
    int64_t m_timestamp;
    uint64_t m_votes;
    std::map<Address, uint64_t> m_voters;
};

extern Producer EmptyProducer;

using ProducerPtr = std::shared_ptr<Producer>;

using Producers = std::vector<Producer>;

using ProducersRef = Producers&;

using ProducersConstRef = Producers const&;

/*
 * ProducerSnapshot: used by producer server
 */
class ProducerSnapshot: public Object {
public:
    ProducerSnapshot() {}

    ProducerSnapshot(ProducerSnapshot const& ps);

    ProducerSnapshot(bytesConstRef data);

    ProducerSnapshot(bytes const& data): ProducerSnapshot(bytesConstRef(&data)) {}

    ProducerSnapshot& operator=(ProducerSnapshot const& ps);

    void populate(bytesConstRef data);

    void populate(bytes const& data) { populate(bytesConstRef(&data)); }

    int64_t getTimestamp() const { return m_timestamp; }

    size_t size() const { return m_producers.size(); }

    ProducersConstRef getProducers() const { return m_producers; }

    void setTimestamp(int64_t timestamp) { m_timestamp = timestamp; }

    void addProducer(Producer const& producer);

    void deleteProducer(Producer const& producer);

    void clear() { m_producers.clear(); }

    void streamRLP(RLPStream& rlpStream) const;

    bool isExist(Address const& address) const;

    virtual bytes getKey() override;

    virtual bytes getRLPData() override;

    virtual Object::ObjectType getObjectType() const override { return Object::ProducerSnapshotType; }

private:
    int64_t m_timestamp;
    Producers m_producers;
};

class ActiveProducerSnapshot: public Object {
public:
    ActiveProducerSnapshot() {}

    ActiveProducerSnapshot(ActiveProducerSnapshot const& ps);

    ActiveProducerSnapshot(bytesConstRef data);

    ActiveProducerSnapshot(bytes const& data): ActiveProducerSnapshot(bytesConstRef(&data)) {}

    ActiveProducerSnapshot& operator=(ActiveProducerSnapshot const& ps);

    void populate(bytesConstRef data);

    void populate(bytes const& data) { populate(bytesConstRef(&data)); }

    int64_t getTimestamp() const { return m_timestamp; }

    size_t size() const { return m_producers.size(); }

    ProducersConstRef getProducers() const { return m_producers; }

    void setTimestamp(int64_t timestamp) { m_timestamp = timestamp; }

    void addProducer(Producer const& producer);

    void deleteProducer(Producer const& producer);

    void clear() { m_producers.clear(); }

    void streamRLP(RLPStream& rlpStream) const;

    bool isExist(Address const& address) const;

    virtual bytes getKey() override;

    virtual bytes getRLPData() override;

    virtual Object::ObjectType getObjectType() const override { return Object::ActiveProducerSnapshotType; }

private:
    int64_t m_timestamp;
    Producers m_producers;
};
} // end of namespace
