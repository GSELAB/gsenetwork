#include <runtime/correlator/Correlation.h>

namespace runtime {

using namespace common;
namespace correlator {

void CorrelatorFilter::relevance(std::vector<std::vector<Transaction>>& txSet)
{
    /*
    std::vector<Transaction> txs;
    for (auto& i : m_block->getTransactions()) {
        txs.push_back(i);
    }

    txSet.push_back(txs);
    */
}

void CorrelatorFilter::run()
{
    /*
    std::vector<std::vector<Transaction>> txSet;
    relevance(txSet);
    std::vector<GVMThread*> tasks;
    for (auto& i : txSet) {
        GVMThread* task = new GVMThread(i, m_block, m_repo, m_count);
        tasks.push_back(task);
        task->start();
    }

    while (m_count.load()) {
        ct++;
    }

    for (auto i : tasks) {
        delete i;
    }
    */

    for (auto& i : m_block->getTransactions()) {
        Runtime runtime(i, m_block, m_repo);
        runtime.init();
        runtime.excute();
        runtime.finished();
    }
}

std::vector<bool> CorrelatorFilter::getResult()
{
    return std::vector<bool>();
}
}
}