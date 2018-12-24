#pragma once

#include <string>

#include <core/Object.h>
#include <core/Address.h>
#include <security/Operation.h>

using namespace core;

namespace runtime {
namespace security {

/*
 * Point to function
 */
struct Strategy {
    std::string m_name;

    std::vector<Operation> m_allowedOperations;

    Strategy() {}

    Strategy(Strategy const& strategy);

    Strategy(std::string const& name);

    Strategy(std::string const& name, std::vector<Operation> const& operations);

    void add(Operation const& operation);

    bool operator==(Strategy const& strategy) const;

    bool operator!=(Strategy const& strategy) const;

    Strategy& operator=(Strategy const& strategy);

    std::string toString() const;
};

struct SEPolicy {
    Address m_contract;

    int64_t m_timestampUsed = 0;

    std::map<std::string, Strategy> m_strategies;

    SEPolicy();

    SEPolicy(SEPolicy const& policy);

    SEPolicy& operator=(SEPolicy const& policy);

    bool operator==(SEPolicy const& policy) const;

    bool operator!=(SEPolicy const& policy) const;

    bool allowed(Address const& user, Address const& target, Operation const& op) const;
};
}
}

