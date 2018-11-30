#pragma once

#include <boost/asio.hpp>
#include <core/Guards.h>
#include <core/Task.h>

namespace chain {

#define SYNC_INTERVAL 2000

class BlockChain;

class Sync: public core::Task {

    enum State {
        Ready = 0x0,
        Running,
        Stop,
    };

    friend class BlockChain;
public:
    Sync(BlockChain* chain): m_chain(chain), m_state(Ready), Task("GSE-Sync") {}

    ~Sync() {

    }

    void start();

    void stop();

    virtual void doWork() override;

    void update(boost::asio::ip::tcp::endpoint const& from, uint64_t height);

private:
    State m_state;
    int64_t m_prevTimestamp = 0;

    BlockChain* m_chain;

    core::Mutex x_heights;
    std::map<boost::asio::ip::tcp::endpoint, uint64_t> m_heights;
};
}