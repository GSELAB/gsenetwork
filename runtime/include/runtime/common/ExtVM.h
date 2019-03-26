#pragma once

#include <storgae/Repository.h>

namespace runtime {

using namespace storage;

namespace vm {

class ExtVM: public VMInterface {
public:
    ExtVM(std::shared_ptr<storage::Repository> repo, Address const& contract, Address const& caller, Address const& origin,
        uint64_t value, bytesConstRef data, bytes const& code, h256 const& codeHash,
        unsigned depth, bool isCreation, bool isStaticCall): VMInterface(contract, caller, origin, value, data,
        code, codeHash, depth, isCreation, isStaticCall) {
            // the contract not allowed beening used
        }

    /// Read storage location.
    u256 store(u256 un) final { return m_s.storage(myAddress, _n); }

    /// Write a value in storage.
    void setStore(u256 _n, u256 _v) final;

    /// Read original storage value (before modifications in the current transaction).
    u256 originalStorageValue(u256 const& _key) final
    {
        return m_s.originalStorageValue(myAddress, _key);
    }

    /// Read address's code.
    bytes const& codeAt(Address _a) final { return m_s.code(_a); }

    /// @returns the size of the code in  bytes at the given address.
    size_t codeSizeAt(Address _a) final;

    /// @returns the hash of the code at the given address.
    h256 codeHashAt(Address _a) final;

    /// Create a new contract.
    CreateResult create(u256 _endowment, u256& io_gas, bytesConstRef _code, Instruction _op, u256 _salt, OnOpFunc const& _onOp = {}) final;

    /// Create a new message call.
    CallResult call(CallParameters& _params) final;

    /// Read address's balance.
    u256 balance(Address _a) final { return m_s.balance(_a); }

    /// Does the account exist?
    bool exists(Address _a) final
    {
        if (evmSchedule().emptinessIsNonexistence())
            return m_s.accountNonemptyAndExisting(_a);
        else
            return m_s.addressInUse(_a);
    }

    /// Suicide the associated contract to the given address.
    void suicide(Address _a) final;

    /// Return the EVM gas-price schedule for this execution context.
    EVMSchedule const& evmSchedule() const final
    {
        return m_sealEngine.evmSchedule(envInfo().number());
    }

    State const& state() const { return m_s; }

    /// Hash of a block if within the last 256 blocks, or h256() otherwise.
    h256 blockHash(u256 _number) final;
};
}
}