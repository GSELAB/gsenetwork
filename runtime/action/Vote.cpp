#include <runtime/action/Vote.h>
#include <core/Ballot.h>
#include <core/Log.h>

using namespace core;

namespace runtime {
namespace action {

void Vote::init()
{

}

void Vote::execute()
{
    Address const& sender = m_transaction.getSender();
    m_repo->clearVote(sender);
    m_repo->burn(sender, PRODUCER_COST);
    bytes const& data = m_transaction.getData();
    bytesConstRef dataRef = &data;
    Ballot ballot(dataRef);
    for (auto const& item : ballot.getCandidateVector()) {
        CINFO << item.getAddress() << ":" << item.getValue();
        m_repo->voteIncrease(sender, item.getAddress(), item.getValue());
    }
}

void Vote::finalize()
{

}
}
}