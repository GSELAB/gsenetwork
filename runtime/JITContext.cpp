
#include <runtime/JITContext.h>

namespace runtime {

JITContext::~JITContext()
{

}

void JITContext::execute()
{

}

void JITContext::cancle()
{

}

void JITContext::requireAuthorization(Address const& address)
{

}

bool JITContext::hasAuthorization(Address const& address)
{
    return true;
}

bool JITContext::addressExist(Address const& address) const
{
    return true;
}

void JITContext::requireRecipient(Address const& address)
{

}

bool JITContext::hasRecipient(Address const& address) const
{
    return true;
}
} // namespace runtime