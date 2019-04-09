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

#include <core/Account.h>
#include <crypto/SHA3.h>

using namespace crypto;
using namespace core;

namespace core {

Account EmptyAccount;

Account::Account(Account const& account)
{
    m_address = account.getAddress();
    m_type = account.getAccountType();
    m_code = account.getCode();
    m_alive = account.isAlive();
    m_balance = account.getBalance();
    m_timestamp = account.getTimestamp();
    for (auto& i : account.getContractAddresses()) {
        m_contractAddresses.push_back(i);
    }

    for (auto i : account.getCandidates())
        m_candidates[i.first] = i.second;
    m_votes = account.getVotes();
}

Account::Account(int64_t timestamp): m_timestamp(timestamp), m_balance(0), m_alive(true), m_votes(0)
{
    // TODO: CREATE A NEW ACCOUNT
}

Account::Account(Address const& contractAddress, AccountType type, int64_t timestamp):
    m_address(contractAddress), m_type(type), m_timestamp(timestamp), m_balance(0), m_alive(true), m_votes(0)
{

}

Account::Account(Address const& address, uint64_t balance, int64_t timestamp): m_address(address), m_balance(balance), m_timestamp(timestamp), m_alive(true), m_votes(0)
{

}

Account::Account(bytesConstRef data)
{
    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() == ACCOUNT_COMMON_FIELDS) {
            m_address = rlp[0].toHash<Address>(RLP::VeryStrict);
            m_type = (AccountType)rlp[1].toInt<uint32_t>();
            m_code = rlp[2].data().toBytes();
            m_alive = rlp[3].toInt<uint8_t>();
            m_balance = rlp[4].toPositiveInt64();
            m_timestamp = rlp[5].toInt<int64_t>();
            bytesConstRef casBytes = rlp[6].data();
            RLP rlpAddresses(casBytes);
            if (rlpAddresses.isList() && rlpAddresses.itemCount() > 0) {
                for (unsigned i = 0; i < rlpAddresses.itemCount(); i++)
                    m_contractAddresses.push_back(rlpAddresses[i].toHash<Address>(RLP::VeryStrict));
            }
            m_votes = rlp[7].toPositiveInt64();
            bytesConstRef csBytes = rlp[8].data();
            RLP rlpCandidates(csBytes);
            if (rlpCandidates.isList() && rlpCandidates.itemCount() > 0) {
                for (unsigned i = 0; i < rlpCandidates.itemCount(); ) {
                    Address address = rlpCandidates[i++].toHash<Address>(RLP::VeryStrict);
                    uint64_t vote = rlpCandidates[i++].toPositiveInt64();
                    m_candidates[address] = vote;
                }
            }
        } else {
            BOOST_THROW_EXCEPTION(std::range_error("Acount RLP must be a list"));
        }
    } catch (Exception e) {
        BOOST_THROW_EXCEPTION(e);
    }
}

Account::~Account()
{

}

Account& Account::operator=(Account const& account)
{
    if (this == &account) return *this;
    m_address = account.getAddress();
    m_type = account.getAccountType();
    m_code = account.getCode();
    m_alive = account.isAlive();
    m_balance = account.getBalance();
    m_timestamp = account.getTimestamp();
    m_candidates.clear();
    for (auto i : account.getCandidates())
        m_candidates[i.first] = i.second;
    m_votes = account.getVotes();
    return *this;
}

bool Account::operator==(Account const& account) const
{
    return (m_address == account.getAddress()) &&
        (m_type == account.getAccountType()) &&
        (m_alive == account.isAlive()) &&
        (m_balance == account.getBalance()) &&
        (m_timestamp == account.getTimestamp());
}

bool Account::equal(Account const& account) const
{
    return (m_address == account.getAddress()) &&
        (m_type == account.getAccountType()) &&
        (m_alive == account.isAlive()) &&
        (m_balance == account.getBalance()) &&
        (m_timestamp == account.getTimestamp());
}

void Account::addVoter(Address const& address, uint64_t value)
{
    m_candidates[address] = value;
    m_votes += value;
}

void Account::clearVote()
{
    m_candidates.clear();
    m_votes = 0;
}

void Account::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(ACCOUNT_COMMON_FIELDS);
    rlpStream << m_address
              << m_type;
    rlpStream << m_code;
    rlpStream << m_alive
              << (bigint)m_balance
              << (bigint)m_timestamp;
    RLPStream contractRLPStream;
    contractRLPStream.appendList(m_contractAddresses.size());
    for (auto i : m_contractAddresses)
        contractRLPStream << i;
    rlpStream.appendRaw(contractRLPStream.out());
    rlpStream << (bigint)m_votes;
    RLPStream candidateRLPStream;
    candidateRLPStream.appendList(m_candidates.size() * 2);
    for (auto i : m_candidates) {
        candidateRLPStream << i.first
                           << (bigint)i.second;
    }
    rlpStream.appendRaw(candidateRLPStream.out());
}

bytes Account::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}
} // end namespace
