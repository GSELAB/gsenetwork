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

namespace core {

Account::Account(int64_t timestamp): m_timestamp(timestamp), m_balance(0), m_alive(true)
{
    // TODO: CREATE A NEW ACCOUNT
}

Account::Account(bytesConstRef data)
{
    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() == ACCOUNT_COMMON_FIELDS) {
            m_address = rlp[0].toHash<Address>(RLP::VeryStrict);
            m_alive = rlp[1].toInt<uint8_t>();
            m_balance = rlp[2].toInt<uint64_t>();
            m_timestamp = rlp[3].toInt<int64_t>();
        } else {
            BOOST_THROW_EXCEPTION(std::range_error("Acount RLP must be a list"));
        }
    } catch (Exception e) {
        //e << errinfo_name("Invalid account format!");
        //throw;
        BOOST_THROW_EXCEPTION(e);
    }
}

Account::~Account()
{

}

bool Account::operator==(Account const& account)
{
    return (m_address == account.getAddress()) &&
        (m_alive == account.isAlive()) &&
        (m_balance == account.getBalance()) &&
        (m_timestamp == account.getTimestamp());
}

void Account::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(ACCOUNT_COMMON_FIELDS);
    rlpStream << m_address
              << m_alive
              << m_balance
              << m_timestamp;
}

void Account::setAddress(Address const& address)
{
    m_address = address;
}

void Account::setAlive(bool status)
{
    m_alive = status;
}

void Account::setBalance(uint64_t balance)
{
    m_balance = balance;
}

void Account::addContractAddress(Address const& address)
{

}

Address const& Account::getAddress() const
{
    return m_address;
}

bool Account::isAlive() const
{
    return m_alive;
}

uint64_t Account::getBalance() const
{
    return m_balance;
}

int64_t Account::getTimestamp() const
{
    return m_timestamp;
}

Addresses const& Account::getContractAddresses() const
{
    return m_contractAddresses;
}

// @override
std::string Account::getKey()
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
std::string Account::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    bytesRef& bsr = *(bytesRef*)&rlpStream.out();
    return bsr.toString();
}
} // end namespace