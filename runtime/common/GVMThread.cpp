#include <runtime/common/GVMThread.h>

namespace runtime {
namespace common {

GVMThread::GVMThread(std::vector<Transaction>& transactions, BlockPtr block, std::shared_ptr<storage::Repository> repo, std::atomic<unsigned int>& count):
    m_transactions(transactions), m_block(block), m_repo(repo), m_count(count)
{

}

void GVMThread::start()
{
    m_count.fetch_add(1);
    m_thread.reset(new std::thread([&] () {
        m_state = GVMState::Started;
        doWork();
        m_state = GVMState::Stoping;
    }));
}

void GVMThread::doWork()
{
    try {
        for (auto& i : m_transactions) {
            Runtime runtime(i, m_block, m_repo);
            runtime.init();
            runtime.excute();
            runtime.finished();
        }

        m_count.fetch_sub(1);
    } catch (std::exception& e) {
        m_count.fetch_sub(1);
        throw e;
    }
}

void GVMThread::terminate()
{
    m_state = GVMState::Killed;
}
}
}