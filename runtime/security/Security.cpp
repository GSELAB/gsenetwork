#include <security/Security.h>
#include <core/Log.h>

using namespace core;

namespace runtime {
namespace security {

class Security: public SecurityInterface {
public:
    Security() = default;

    virtual ~Security() {}

    virtual void add(SEPolicy const& policy);

    virtual bool allowOperation(Address const& user, Address const& target, Operation const& op);

protected:
    void doCheck();

private:
    PolicyIndex m_policyIndex;
};

void Security::doCheck()
{
    if (m_policyIndex.size() > MAX_CACHE_POLICY) {
        auto& idx = m_policyIndex.get<SETIME>();
        auto itr = m_policyIndex.find(idx.begin()->m_contract);
        m_policyIndex.erase(itr);
    }
}

void Security::add(SEPolicy const& policy)
{
    auto ret = m_policyIndex.insert(policy);
    if (!ret.second) {
        CDEBUG << "Duplicate policy - " << policy.m_contract;
        auto itr = m_policyIndex.find(policy.m_contract);
        m_policyIndex.modify(itr, [&] (auto& item) {
            item.m_timestampUsed = policy.m_timestampUsed;
        });
    }

    doCheck();
}

bool Security::allowOperation(Address const& user, Address const& target, Operation const& op)
{
    auto& idx = m_policyIndex.get<SEID>();
    auto itr = idx.find(target);
    if (itr == idx.end())
        return true;

    return itr->allowed(user, target, op);
}
}
}