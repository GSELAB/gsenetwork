#include <runtime/action/BeenProducer.h>
#include <core/Producer.h>

using namespace core;

namespace runtime {
namespace action {

// @override
void BeenProducer::init()
{

}

// @override
void BeenProducer::execute()
{
    // Burn 1024 GSE Token
    Address const& sender = m_transaction.getSender();
    if (!m_repo->burn(sender, 1024))
        return;

    Producer producer(sender, m_block->getBlockHeader().getTimestamp());
    m_repo->put(producer);
}

// @override
void BeenProducer::finalize()
{

}

}
}