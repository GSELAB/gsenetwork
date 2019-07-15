#include <security/SEPolicy.h>

namespace runtime {
namespace security {

Strategy::Strategy(Strategy const& strategy)
{
    m_name = strategy.m_name;
    for (auto& i : strategy.m_allowedOperations)
        m_allowedOperations.push_back(i);
}

Strategy::Strategy(std::string const& name): m_name(name)
{

}

Strategy::Strategy(std::string const& name, std::vector<Operation> const& operations): m_name(name)
{
    for (auto& i : operations)
        m_allowedOperations.push_back(i);
}

void Strategy::add(Operation const& operation)
{
    m_allowedOperations.push_back(operation);
}

bool Strategy::operator==(Strategy const& strategy) const
{
    if (m_name != strategy.m_name || m_allowedOperations.size() != strategy.m_allowedOperations.size())
        return false;

    return true;
}

bool Strategy::operator!=(Strategy const& strategy) const
{
    return !operator==(strategy);
}

Strategy& Strategy::operator=(Strategy const& strategy)
{
    if (this == &strategy)
        return *this;

    m_name = strategy.m_name;
    m_allowedOperations.clear();
    for (auto& i : strategy.m_allowedOperations)
        m_allowedOperations.push_back(i);

    return *this;
}

std::string Strategy::toString() const
{
    return std::string();
}

SEPolicy::SEPolicy()
{

}

SEPolicy::SEPolicy(SEPolicy const& policy)
{
    m_contract = policy.m_contract;
    m_timestampUsed = policy.m_timestampUsed;
    for (auto& i : policy.m_strategies)
        m_strategies[i.first] = i.second;
}

SEPolicy& SEPolicy::operator=(SEPolicy const& policy)
{
    if (this == &policy)
        return *this;

    m_contract = policy.m_contract;
    m_timestampUsed = policy.m_timestampUsed;
    m_strategies.clear();
    for (auto& i : policy.m_strategies)
        m_strategies[i.first] = i.second;

    return *this;
}

bool SEPolicy::operator==(SEPolicy const& policy) const
{
    return false;
}

bool SEPolicy::operator!=(SEPolicy const& policy) const
{
    return true;
}

bool SEPolicy::allowed(Address const& user, Address const& target, Operation const& op) const
{
    return true;
}
}
}