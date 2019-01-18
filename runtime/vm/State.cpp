#include <vm/State.h>

namespace runtime {
namespace vm {

State::State(std::shared_ptr<storage::Repository> repo): m_repo(repo)
{

}

bool State::addressHasCode(Address const& address)
{

}

uint64_t State::balance(Address const& address)
{

}

void State::addBalance(Address const& address, uint64_t amount)
{

}

void State::subBalance(Address const& address, uint64_t amount)
{

}

void State::setBalance(Address const& address, uint64_t value)
{

}

void State::transfer(Address const& from, Address const& to, uint64_t amount)
{

}

/// Get the value of a storage position of an account.  @returns 0 if no account exists at that address.
u256 State::storage(Address const& contract, u256 const& memory)
{

}

void State::setStorage(Address const& contract, u256 const& location, u256 const& value)
{

}

u256 State::originalStorageValue(Address const& contract, u256 const& key)
{}

void State::clearStorage(Address const& contract)
{

}

void State::createContract(Address const& address)
{

}

void State::setCode(Address const& contract, bytes&& code)
{

}

/// Delete an account (used for processing suicides).
void State::kill(Address const& address)
{

}

std::map<h256, std::pair<u256, u256>> State::storage(Address const& contract)
{

}

bytes State::code(Address const& address)
{

}

h256 State::codeHash(Address const& address)
{

}

size_t State::codeSize(Address const& address)
{

}

Account State::getAccount(Address const& address)
{

}

void State::putAccount(Account const& account)
{

}
}
}