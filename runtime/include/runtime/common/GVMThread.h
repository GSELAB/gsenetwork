#pragma once

#include <thread>
#include <atomic>

#include <core/Block.h>
#include <core/Transaction.h>

#include <runtime/common/Runtime.h>

using namespace core;

namespace runtime {
namespace common {

class GVMThread {
public:
    enum GVMState: uint32_t {
        Starting    = 0x0,
        Started     = 0x01,
        Stoping     = 0x02,
        Stoped      = 0x03,
        Killed      = 0x04
    };

public:
    GVMThread(std::vector<Transaction>& transactions, BlockPtr block, std::shared_ptr<storage::Repository> repo, std::atomic<unsigned int>& count);

    virtual ~GVMThread() {}

    void start();

    void doWork();

    void terminate();

private:
    std::atomic<unsigned int>& m_count;

    std::vector<Transaction>& m_transactions;

    std::shared_ptr<storage::Repository> m_repo;

    BlockPtr m_block;

    std::atomic<GVMState> m_state = { GVMState::Starting };

    std::unique_ptr<std::thread> m_thread;
};
}
}