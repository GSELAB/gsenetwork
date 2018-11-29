#include <runtime/action/ActionFactory.h>
#include <runtime/action/Vote.h>
#include <runtime/action/Transfer.h>
#include <runtime/action/BeenProducer.h>

using namespace core;

namespace runtime {
namespace action {

ActionFactory::ActionFactory(Transaction const& transaction, BlockPtr block, std::shared_ptr<Repository> repo)
{
    switch (transaction.getType()) {
        case Transaction::VoteType:
            m_handler = new Vote(transaction, block, repo);
            break;
        case Transaction::TransferType:
            m_handler = new Transfer(transaction, block, repo);
            break;
        case Transaction::BeenProducerType:
            m_handler = new BeenProducer(transaction, block, repo);
            break;
        default:
            m_handler = nullptr;
            break;
    }
}

ActionFactory::~ActionFactory()
{
    if (m_handler)
        delete m_handler;
}

void ActionFactory::init()
{
    if (m_handler)
        m_handler->init();
}

void ActionFactory::execute()
{
    if (m_handler)
        m_handler->execute();
}

void ActionFactory::finalize()
{
    if (m_handler)
        m_handler->finalize();
}

}
}