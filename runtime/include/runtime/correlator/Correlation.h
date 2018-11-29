#pragma once

#include <core/Block.h>
#include <core/Transaction.h>
#include <runtime/common/GVMThread.h>

using namespace core;

namespace runtime {
namespace correlator {

#define MAX_FILTER_THREAD   20

class CorrelatorFilter {
public:
    CorrelatorFilter() = delete;

    CorrelatorFilter(BlockPtr block, std::shared_ptr<storage::Repository> repo): m_block(block), m_repo(repo) {}

    virtual ~CorrelatorFilter() {}

    void run();

    std::vector<bool> getResult();

private:
    void relevance(std::vector<std::vector<Transaction>>& txSet);

private:
    BlockPtr m_block;

    std::shared_ptr<storage::Repository> m_repo;

    std::atomic<unsigned int> m_count = { 0 };
};
}
}