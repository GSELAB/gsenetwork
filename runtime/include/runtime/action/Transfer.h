#pragma once

#include <runtime/action/ActionHandler.h>
#include <core/Transaction.h>
#include <storage/Repository.h>

using namespace core;

namespace runtime {

using namespace storage;

namespace action {

class Transfer: public ActionHandler {
public:
    Transfer(Transaction const& transaction, BlockPtr block, std::shared_ptr<Repository> repo):
        m_transaction(transaction), m_block(block), m_repo(repo) {}

    ~Transfer() {}

    virtual void init() override;

    virtual void execute() override;

    virtual void finalize() override;

private:
    Transaction m_transaction;
    BlockPtr m_block;
    std::shared_ptr<Repository> m_repo;
};
}
}