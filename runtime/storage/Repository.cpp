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

#include <storage/Repository.h>
#include <core/Log.h>

using namespace core;

namespace runtime {
namespace storage {

#define GET_FROM_CURRENT_RETURN(name, arg, ret) \
do {    \
    Guard l(x_mutex##name); \
    auto item = m_cache##name.find(arg);    \
    if (item != m_cache##name.end()) return item->second;   \
} while(0)

#define GET_FROM_PARENT_RETURN(name, arg, ret)  \
do {    \
    if (m_parent) { \
        ret = m_parent->get##name(arg); \
        if (ret == Empty##name) {} else { put(ret); return ret; }   \
    }   \
} while(0)

#define GET_FROM_DBC_RETURN(name, arg, ret)     \
do {    \
    ret = m_dbc->get##name(arg);    \
    if (ret == Empty##name) {} else { put(ret); return ret;}    \
} while(0)

#define COMMIT_REPO(name)   \
do {    \
    Guard l{x_mutex##name}; \
    if (m_parent) { for (auto i : m_cache##name) m_parent->put(i.second); } else { for (auto i : m_cache##name) m_dbc->put(i.second); }  \
} while (0)

Account Repository::getAccount(Address const& address)
{
    Account account(EmptyAccount);
    GET_FROM_CURRENT_RETURN(Account, address, account);
    GET_FROM_PARENT_RETURN(Account, address, account);
    GET_FROM_DBC_RETURN(Account, address, account);
    return EmptyAccount;
}

void Repository::put(Account const& account)
{
    Guard l(x_mutexAccount);
    auto item = m_cacheAccount.find(account.getAddress());
    if (item != m_cacheAccount.end()) {
        m_cacheAccount[account.getAddress()] = account; /// update
    } else {
        m_cacheAccount.emplace(account.getAddress(), account);  /// insert
    }
}

bool Repository::transfer(Address const& from, Address const& to, uint64_t value)
{
    Account _from = getAccount(from);
    Account _to = getAccount(to);

    if (_from == EmptyAccount) {
        //std::count << "from account is null";
        return false;
    }

    if (_from.getBalance() < value || _to.getBalance() + value < value)
        return false;

    _from.setBalance(_from.getBalance() - value);
    _to.setBalance(_to.getBalance() + value);
    put(_from);
    put(_to);
    return true;
}

bool Repository::burn(Address const& target, uint64_t value)
{
    Account _target = getAccount(target);
    if (_target == EmptyAccount) {
        CERROR << "Burn token from empty account";
        return false;
    }

    if (_target.getBalance() < value) {
        CERROR << "Not enough token for burning";
        return false;
    }

    _target.setBalance(_target.getBalance() - value);
    put(_target);
    return true;
}

Producer Repository::getProducer(Address const& address)
{
    Producer producer;
    GET_FROM_CURRENT_RETURN(Producer, address, producer);
    GET_FROM_PARENT_RETURN(Producer, address, producer);
    GET_FROM_DBC_RETURN(Producer, address, producer);
    return EmptyProducer;
}

void Repository::put(Producer const& producer)
{
    Guard l(x_mutexProducer);
    auto item = m_cacheProducer.find(producer.getAddress());
    if (item != m_cacheProducer.end()) {
        m_cacheProducer[producer.getAddress()] = producer; /// update
    } else {
        m_cacheProducer.emplace(producer.getAddress(), producer);
    }
}

Transaction Repository::getTransaction(TxID const& id)
{
    Transaction tx;
    {
        Guard l(x_mutexTransaction);
        auto itr = m_cacheTransaction.find(id);
        if (itr != m_cacheTransaction.end())
            return itr->second;
    }

    if (m_parent) {
        tx = m_parent->getTransaction(id);
        if (tx == EmptyTransaction) {} else {
            put(tx);
            return tx;
        }
    }

    tx = m_dbc->getTransaction(id);
    if (tx == EmptyTransaction) {} else {
        put(tx);
        return tx;
    }

    return EmptyTransaction;
}

void Repository::put(Transaction& tx)
{
    Guard l(x_mutexTransaction);
    h256 hash = tx.getHash();
    auto itr = m_cacheTransaction.find(hash);
    if (itr != m_cacheTransaction.end()) {
        m_cacheTransaction[hash] = tx;
    } else {
        m_cacheTransaction.emplace(hash, tx);
    }
}

void Repository::put(Block const& block)
{
    // DO NOTHING
}

void Repository::voteIncrease(Address const& voter, Address const& candidate, uint64_t value)
{
    Account account = getAccount(voter);
    if (account == EmptyAccount) {
        CERROR << "Voter account not exist";
        throw VoteNotExistAccountException("Voter account not exist");
    }

    Producer producer = getProducer(candidate);
    if (producer == EmptyProducer) {
        CERROR << "Vote not exist producer";
        throw VoteNotExistProducerException("Vote not exist producer");
    }

    // TODO:
    //producer
}

void Repository::voteDecrease(Address const& voter, Address const& candidate, uint64_t value)
{
    Producer producer = getProducer(candidate);
    if (producer == EmptyProducer) {
        CERROR << "Vote not exist producer";
        throw VoteNotExistProducerException("Vote not exist producer");
    }

    // TODO
}

void Repository::commit()
{
    // commit to db?
    COMMIT_REPO(Account);
    COMMIT_REPO(Producer);
    COMMIT_REPO(Transaction);
    if (!m_parent)
        m_dbc->put(getBlock());
}

} // end namespace storage
} // end namespace runtime