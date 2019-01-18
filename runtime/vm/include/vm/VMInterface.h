#pragma once

#include <storage/Repository.h>
#include <core/Account.h>

namespace runtime {

using namespace storage;

namespace vm {

class VMInterface {
public:
    VMInterface(Address const& contract, Address const& caller, Address const& origin,
        uint64_t value, bytesConstRef data, bytes const& code, h256 const& codeHash,
        unsigned depth, bool isCreation, bool isStaticCall);

    virtual ~VMInterface() = default;

    VMInterface(VMInterface const&) = delete;

    VMInterface& operator=(VMInterface const&) = delete;

    /// Read storage location.
    virtual u256 store(u256) { return 0; }

    /// Write a value in storage.
    virtual void setStore(u256, u256) {}

    /// Read original storage value (before modifications in the current transaction).
    virtual u256 originalStorageValue(u256 const&) { return 0; }

    /// Read address's balance.
    virtual u256 balance(Address) { return 0; }

    /// Read address's code.
    virtual bytes const& codeAt(Address) { return NullBytes; }

    /// @returns the size of the code in bytes at the given address.
    virtual size_t codeSizeAt(Address) { return 0; }

    /// @returns the hash of the code at the given address.
    virtual h256 codeHashAt(Address) { return h256{}; }

    /// Does the account exist?
    virtual bool exists(Address) { return false; }

    /// Suicide the associated contract and give proceeds to the given address.
    virtual void suicide(Address) { sub.suicides.insert(myAddress); }

    /// Create a new (contract) account.
    virtual CreateResult create(u256, u256&, bytesConstRef, Instruction, u256, OnOpFunc const&) = 0;

    /// Make a new message call.
    virtual CallResult call(CallParameters&) = 0;

    /// Hash of a block if within the last 256 blocks, or h256() otherwise.
    virtual h256 blockHash(u256 number) = 0;

    /// Return the EVM gas-price schedule for this execution context.
    virtual EVMSchedule const& evmSchedule() const { return DefaultSchedule; }

public:
    Address m_contract;  ///< Address associated with executing code (a contract, or contract-to-be).
    Address m_caller;     ///< Address which sent the message (either equal to origin or a contract).
    Address m_origin;     ///< Original transactor.
    u256 m_value;         ///< Value (in Wei) that was passed to this address.
    bytesConstRef m_data;       ///< Current input data.
    bytes m_code;               ///< Current code that is executing.
    h256 m_codeHash;            ///< SHA3 hash of the executing code
    u256 m_salt;                ///< Values used in new address construction by CREATE2
    SubState m_sub;             ///< Sub-band VM state (suicides, refund counter, logs).
    unsigned m_depth = 0;       ///< Depth of the present call.
    bool m_isCreation = false;    ///< Is this a CREATE call?
    bool m_staticCall = false;  ///< Throw on state changing.

};
}
}