#pragma once

#include <core/Account.h>
#include <storage/Repository.h>

using namespace core;

namespace runtime {

using namespace storage;

namespace vm {

class State {
public:
    State(std::shared_ptr<storage::Repository> repo);

    virtual ~State() = default;

    State& operator=(State const&) = delete;

    bool addressHasCode(Address const& address);

    uint64_t balance(Address const& address);

    void addBalance(Address const& address, uint64_t amount);

    void subBalance(Address const& address, uint64_t amount);

    void setBalance(Address const& address, uint64_t value);

    void transfer(Address const& from, Address const& to, uint64_t amount);

    /// Get the value of a storage position of an account.  @returns 0 if no account exists at that address.
    u256 storage(Address const& contract, u256 const& memory);

    void setStorage(Address const& contract, u256 const& location, u256 const& value);

    u256 originalStorageValue(Address const& contract, u256 const& key);

    void clearStorage(Address const& contract);

    void createContract(Address const& address);

    void setCode(Address const& contract, bytes&& code);

    /// Delete an account (used for processing suicides).
    void kill(Address const& address);

    std::map<h256, std::pair<u256, u256>> storage(Address const& contract);

    bytes code(Address const& address);

    h256 codeHash(Address const& address);

    size_t codeSize(Address const& address);

public:
    Account getAccount(Address const& address);

    void putAccount(Account const& account);

private:
    std::shared_ptr<storage::Repository> m_repo;
};
}
}