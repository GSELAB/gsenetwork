#pragma once

#include <runtime/action/ActionHandler.h>
#include <core/Transaction.h>
#include <storage/Repository.h>

using namespace core;

namespace runtime {

using namespace storage;

namespace action {

class Vote: public ActionHandler {
public:
    Vote(Transaction transaction, Block* block, std::shared_ptr<Repository> repo):
        m_transaction(transaction), m_block(block), m_repo(repo) {}

    ~Vote() {}

    virtual void init() override;

    virtual void execute() override;

    virtual void finalize() override;

private:
    Transaction m_transaction;
    Block* m_block;
    std::shared_ptr<Repository> m_repo;
};
}
}