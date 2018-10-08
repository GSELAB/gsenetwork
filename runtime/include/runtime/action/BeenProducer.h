#pragma once

#include <runtime/action/ActionHandler.h>
#include <core/Transaction.h>
#include <storage/Repository.h>

using namespace core;

namespace runtime {

using namespace storage;

namespace action {

class BeenProducer: public ActionHandler {
public:
    BeenProducer(Transaction transaction, std::shared_ptr<Repository> repo):
        m_transaction(transaction), m_repo(repo) {}

    ~BeenProducer() {}

    // @override
    void init();

    // @override
    void execute();

    // @override
    void finalize();

private:
    Transaction m_transaction;
    std::shared_ptr<Repository> m_repo;
};
}
}