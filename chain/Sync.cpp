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
    if (isWorking())
        stopWorking();
    m_state = Stop;
    terminate();
}

bool heightCompare(std::pair<boost::asio::ip::tcp::endpoint, uint64_t> const& p1, std::pair<boost::asio::ip::tcp::endpoint, uint64_t> const& p2)
{
    return p1.second < p2.second;
}

void Sync::doWork()
{
    if (m_state != Running)
        return;

    int64_t ct = currentTimestamp();
    if (m_chain->getBlockChainStatus() == BlockChainStatus::SyncStatus || ((ct - m_prevTimestamp) >= SYNC_INTERVAL)) {
        m_prevTimestamp = ct;
        if (isWorking()) {
            sleepMilliseconds(SYNC_INTERVAL / 5);
        }
    } else {
        if (isWorking()) {
            sleepMilliseconds(SYNC_INTERVAL);
        }
        return;
    }

    std::vector<std::pair<boost::asio::ip::tcp::endpoint, uint64_t>> heights;
    {
        Guard l{x_heights};
        for (auto i : m_heights) {
            heights.push_back(std::make_pair(i.first, i.second));
        }
    }

    if (heights.empty()) {
        m_chain->m_blockChainStatus = SyncStatus;
        return;
    }

    if (heights.size() > 1) {
        std::sort(heights.begin(), heights.end(), heightCompare);
    }

#define COUNT_PER_ROUND 100
    unsigned idx = 0;
    bool needSync = false;
    uint64_t requestStart, requestEnd;
    uint64_t currentHeight = m_chain->getLastBlockNumber();
    requestStart = currentHeight + 1;
    for (auto& i : heights) {
        if ((idx % 2) == 0) {
            if (i.second >= requestStart) {
                if (i.second - requestStart >= COUNT_PER_ROUND) {
                    requestEnd = requestStart + COUNT_PER_ROUND - 1;
                } else {
                    requestEnd = i.second;
                }
            } else {
                continue;
            }
        }

        needSync = true;
        m_chain->m_blockChainStatus = SyncStatus;
        Status status(SyncBlocks, requestStart, requestEnd);
        m_chain->m_messageFace->send(i.first, status);
        /// CINFO << "Sync request block from " << i.first << " (" << requestStart << " - " << requestEnd << ")";
        if ((idx % 2) == 1) {
            requestStart = requestStart + COUNT_PER_ROUND;
        }
        idx++;
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