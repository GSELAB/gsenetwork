#include <runtime/action/Vote.h>
#include <core/Ballot.h>
#include <core/Log.h>

using namespace core;

namespace runtime {
namespace action {

// @override
void Vote::init()
{

}

// @override
void Vote::execute()
{
    Address const& sender = m_transaction.getSender();
    bytes const& data = m_transaction.getData();
    bytesConstRef dataRef = &data;
    Ballot ballot(dataRef);
    for (auto const& item : ballot.getCandidateVector()) {
        CINFO << item.getAddress() << ":" << item.getValue();
        m_repo->voteIncrease(sender, item.getAddress(), item.getValue());
    }



}

// @override
void Vote::finalize()
{

}
}
}