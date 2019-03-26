#pragma once

#include <core/Block.h>
#include <core/Transaction.h>
#include <storage/Repository.h>

namespace runtime {

using namespace storage;

namespace vm {

class Runtime {
public:
    enum Type {
        PreType = 0x0,
        NormalType = 0x01,
    };

    Runtime(BlockPtr block, Transaction& tx, std::shared_ptr<storage::Repository> repo):
        m_block(block), m_tx(tx), m_repo(repo), m_type(NormalType) {}

    // @just for pre execute
    Runtime(Transaction const& tx, std::shared_ptr<storage::Repository> repo):
        m_block(std::make_shared<Block>()), m_tx(tx), m_repo(repo), m_type(PreType) {}

    void init();

    void execute();

    bool create();

    bool call();

    bool go();

    void revert();

private:
    Type m_type;

    Transaction m_tx;

    BlockPtr m_block;

    std::shared_ptr<Repository> m_repo;

    unsigned m_depth = 0;

    Address m_creationAddress;

    bool m_isCreation = false;
};
}
}