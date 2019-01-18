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
    Account account;
    GET_FROM_CURRENT_RETURN(Account, address, account);
    GET_FROM_PARENT_RETURN(Account, address, account);
    GET_FROM_DBC_RETURN(Account, address, account);
    return account;
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

void Repository::transfer(Address const& from, Address const& to, uint64_t value)
{
    Account _from = getAccount(from);
    Account _to = getAccount(to);
    if (_from == EmptyAccount) {
        throw RepositoryException("transfer - from:" + toString(from) + " is empty.");
    }

    if (_to == EmptyAccount) {
        Account account(to, 0, m_block.getBlockHeader().getTimestamp());
        _to = account;
        /// CINFO << "Create account - " << to;
    }

    if (_from.getBalance() < value || _to.getBalance() + value < value) {
        throw RepositoryException("transfer - from:" + toString(from) + " to:" + toString(to) + " not enough balance or overflow.");
    }

    _from.setBalance(_from.getBalance() - value);
    _to.setBalance(_to.getBalance() + value);
    put(_from);
    put(_to);
}

void Repository::burn(Address const& target, uint64_t value)
{
    Account _target = getAccount(target);
    if (_target == EmptyAccount) {
        throw RepositoryException("burn - " + toString(target) + " is empty.");
    }

    if (_target.getBalance() < value) {
        throw RepositoryException("burn - " + toString(target) + " not enough balance.");
    }

    _target.setBalance(_target.getBalance() - value);
    put(_target);
}

void Repository::bonus(Address const& target, uint64_t value)
{
    Account _target = getAccount(target);
    if (_target == EmptyAccount) {
        throw RepositoryException("reward - " + toString(target) + " is empty.");
    }

    _target.setBalance(_target.getBalance() + value);
    put(_target);
}

Producer Repository::getProducer(Address const& address)
{
    Producer producer;
    GET_FROM_CURRENT_RETURN(Producer, address, producer);
    GET_FROM_PARENT_RETURN(Producer, address, producer);
    GET_FROM_DBC_RETURN(Producer, address, producer);
    return EmptyProducer;
}

std::map<Address, Producer> Repository::getProducerList() const
{
    std::map<Address, Producer> ret;
    if (m_parent != nullptr) {
        ret = m_parent->getProducerList();
    } else {
        ret = m_dbc->getProducerList();
    }

    {
        Guard l{x_mutexProducer};
        for (auto const& i : m_cacheProducer) {
            ret[i.first] = i.second;
        }
    }

    return ret;
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

Block Repository::getBlock(BlockID const& blockID)
{
    Block ret;
    if (m_block.getHash() == blockID)
        return m_block;

    if (m_parent) {
        ret = m_parent->getBlock(blockID);
        if (ret == EmptyBlock) {} else {
            return ret;
        }
    }

    ret = m_dbc->getBlock(blockID);
    return ret;
}

void Repository::clearVote(Address const& address)
{
    Account account = getAccount(address);
    if (account == EmptyAccount) {
        CERROR << "Clear vote account (" << address << ") not exist";
        throw RepositoryException("Clear vote account (" + toString(address) + ") not exist");
    }

    for (auto i : account.getCandidates()) {
        Producer producer = getProducer(i.first);
        if (producer == EmptyProducer) {
            CERROR << "Producer (" << i.first << ") not exist";
            throw RepositoryException("Producer (" + toString(i.first) + ") not exist");
        }

        producer.eraseVoter(address);
        put(producer);
    }

    account.clearVote();
    put(account);
}

void Repository::voteIncrease(Address const& voter, Address const& candidate, uint64_t value)
{
    Account account = getAccount(voter);
    if (account == EmptyAccount) {
        CERROR << "Voter account not exist";
        throw RepositoryException("Voter account not exist");
    }

    Producer producer = getProducer(candidate);
    if (producer == EmptyProducer) {
        CERROR << "Vote not exist producer";
        throw RepositoryException("Vote not exist producer");
    }

    if ((account.getBalance() - account.getVotes() * 1000000) < value * 1000000) {
        CERROR << "Voter (" << voter << ") has not enough balance";
        throw RepositoryException("Voter (" + toString(voter) + ") has not enough balance");
    }

    account.addVoter(candidate, value);
    producer.addVoter(voter, value);
    put(account);
    put(producer);
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

bool Repository::isAddressExist(Address const& address)
{
    Account account = getAccount(address);
    if (account == EmptyAccount) {
        return false;
    }

    return true;
}

bool Repository::isContractAddress(Address const& address)
{
    Account account = getAccount(address);
    if (account == EmptyAccount) {
        return false;
    }

    if (account.getAccountType() == Account::ContractType) {
        return true;
    }

    return false;
}

} // end namespace storage
} // end namespace runtime
