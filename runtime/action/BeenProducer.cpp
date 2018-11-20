#include <runtime/action/BeenProducer.h>
#include <core/Producer.h>

using namespace core;

namespace runtime {
namespace action {

void BeenProducer::init()
{

}

void BeenProducer::execute()
{
    // Burn 1024 GSE Token
    Address const& sender = m_transaction.getSender();
    m_repo->burn(sender, TRANSACTION_FEE);
    m_repo->burn(sender, PRODUCER_COST);
    Producer producer(sender, m_block->getBlockHeader().getTimestamp());
    producer.setVotes(0);
    m_repo->put(producer);
}

void BeenProducer::finalize()
{

}
}
}