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
#include <core/RLP.h>
#include <core/Common.h>
#include <core/Address.h>
#include <config/Constant.h>

using namespace config;

namespace core {

#define ACCOUNT_COMMON_FIELDS (7)

class Account: public Object {
public:
    Account() {}

    Account(Account const& account);

    Account(Address const& address, uint64_t balance, int64_t timestamp = GENESIS_TIMESTAMP);

    Account(int64_t timestamp);

    Account(bytesConstRef data);

    ~Account();

    Account& operator=(Account const& account);

    bool operator==(Account const& account) const;

    bool equal(Account const& account) const;

    void streamRLP(RLPStream& rlpStream) const;

    void setAddress(Address const& address) { m_address = address; }

    void setAlive(bool status) { m_alive = status; }

    void setBalance(uint64_t balance) { m_balance = balance; }

    void addContractAddress(Address const& address) {}

    void addVoter(Address const& address, uint64_t value);

    void clearVote();

    Address const& getAddress() const { return m_address; }

    bool isAlive() const { return m_alive; }

    uint64_t getBalance() const { return m_balance; }

    int64_t getTimestamp() const { return m_timestamp; }

    Addresses const& getContractAddresses() const { return m_contractAddresses; }

    std::map<Address, uint64_t> const& getCandidates() const { return m_candidates; }

    uint64_t getVotes() const { return m_votes; }

    virtual bytes getKey() override { return m_address.asBytes(); }

    virtual bytes getRLPData() override;

    virtual Object::ObjectType getObjectType() const override { return Object::AccountType; }

private:
    Address m_address;

    bool m_alive = false;

    uint64_t m_balance;

    int64_t m_timestamp;

    Addresses m_contractAddresses;

    uint64_t m_votes;

    std::map<Address, uint64_t> m_candidates;
};

extern Account EmptyAccount;

} /* namespace */