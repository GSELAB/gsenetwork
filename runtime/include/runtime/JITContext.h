#pragma once

#include <crypto/Common.h>
#include <storage/Repository.h>

class Controller;
extern Controller controller;

using namespace core;

namespace runtime {

class JITContext {
public:
    JITContext(Transaction const& transaction, uint32_t depth = 0): m_transaction(transaction) {}

    ~JITContext();

    void execute();

    void cancle();

    void requireAuthorization(Address const& address);

    bool hasAuthorization(Address const& address);

    bool addressExist(Address const& address) const;

    void requireRecipient(Address const& address);

    bool hasRecipient(Address const& address) const;

public:
    Controller* m_controller;
    std::shared_ptr<storage::Repository> m_repository;


private:
    Transaction const& m_transaction;
    Address m_receiver;
    std::vector<Address> m_notified;


};
}