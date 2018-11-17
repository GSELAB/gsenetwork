#include <chain/Sync.h>
#include <chain/BlockChain.h>
#include <chain/Common.h>
#include <utils/Utils.h>
#include <core/Status.h>

using namespace core;
using namespace utils;

namespace chain {

void Sync::start()
{
    startWorking();
    if (isWorking()) {
        m_state = Running;
        return;
    }
    doneWorking();
}

void Sync::stop()
{
    if (isWorking()) {
        stopWorking();
    }
    m_state = Stop;
}

void Sync::doWork()
{
    if (m_state != Running)
        return;

    int64_t ct = currentTimestamp();
    if (m_chain->getBlockChainStatus() == BlockChainStatus::SyncStatus || ((ct - m_prevTimestamp) >= SYNC_INTERVAL)) {
        m_prevTimestamp = ct;
        sleepMilliseconds(SYNC_INTERVAL / 10);
    } else {
        sleepMilliseconds(SYNC_INTERVAL);
        return;
    }

    std::map<boost::asio::ip::tcp::endpoint, uint64_t> heights;
    {
        Guard l{x_heights};
        for (auto i : m_heights) {
            heights[i.first] = i.second;
        }
    }

    if (heights.empty()) {
        m_chain->m_blockChainStatus = SyncStatus;
        return;
    }

    bool needSync = false;
    uint64_t currentHeight = m_chain->getLastBlockNumber();
    for (auto i : heights) {
        if (i.second > currentHeight) {
            uint64_t requestStart, requestEnd;
            requestStart = currentHeight + 1;
            if (i.second - currentHeight > 10) {
                requestEnd = currentHeight + 10;
            } else {
                requestEnd = i.second;
            }
            CINFO << "Sync request block from " << i.first << " (" << requestStart << " - " << requestEnd << ")";
            needSync = true;
            m_chain->m_blockChainStatus = SyncStatus;
            Status status(SyncBlocks, requestStart, requestEnd);
            m_chain->m_messageFace->send(i.first, status);
        }
    }

    if (needSync == false && m_chain->m_blockChainStatus == SyncStatus) {
        m_chain->m_blockChainStatus = ProducerStatus;
    }
}

void Sync::update(boost::asio::ip::tcp::endpoint const& from, uint64_t height)
{
    Guard l{x_heights};
    m_heights[from] = height;
}

} // namespace chain