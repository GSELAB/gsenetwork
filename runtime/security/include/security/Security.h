#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <core/Address.h>
#include <security/SEPolicy.h>
#include <security/Operation.h>

using namespace boost::multi_index;

using namespace core;

namespace runtime {
namespace security {

#define MAX_CACHE_POLICY   256

struct SEID {};
struct SETIME {};

typedef boost::multi_index::multi_index_container<
    SEPolicy,
    indexed_by<
        ordered_unique<tag<SEID>, BOOST_MULTI_INDEX_MEMBER(SEPolicy, Address, m_contract)>,
        ordered_non_unique<tag<SETIME>, BOOST_MULTI_INDEX_MEMBER(SEPolicy, int64_t, m_timestampUsed)>
    >
> PolicyIndex;

class SecurityInterface {
public:
    virtual void add(SEPolicy const& policy) = 0;

    virtual bool allowOperation(Address const& user, Address const& target, Operation const& op) = 0;
};
}
}