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
    Transfer(Transaction const& transaction, Block* block, std::shared_ptr<Repository> repo):
        m_transaction(transaction), m_block(block), m_repo(repo) {}

    ~Transfer() {}

    // @override
    void init();

    // @override
    void execute();

    // @override
    void finalize();

private:
    Transaction m_transaction;
    Block* m_block;
    std::shared_ptr<Repository> m_repo;
};
}
}