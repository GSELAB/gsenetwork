#pragma once

#include <storage/Repository.h>
#include <core/Account.h>
#include <vm/Instruction.h>
#include <vm/Config.h>
#include <vm/Types.h>

namespace runtime {

using namespace storage;

namespace vm {

class owning_bytes_ref: public vector_ref<byte const>
{
public:
    owning_bytes_ref() = default;

    /// @param _bytes  The buffer.
    /// @param _begin  The index of the first referenced byte.
    /// @param _size   The number of referenced bytes.
    owning_bytes_ref(bytes&& _bytes, size_t _begin, size_t _size):
            m_bytes(std::move(_bytes))
    {
        // Set the reference *after* the buffer is moved to avoid
        // pointer invalidation.
        retarget(&m_bytes[_begin], _size);
    }

    owning_bytes_ref(owning_bytes_ref const&) = delete;
    owning_bytes_ref(owning_bytes_ref&&) = default;
    owning_bytes_ref& operator=(owning_bytes_ref const&) = delete;
    owning_bytes_ref& operator=(owning_bytes_ref&&) = default;

    /// Moves the bytes vector out of here. The object cannot be used any more.
    bytes&& takeBytes()
    {
        reset();  // Reset reference just in case.
        return std::move(m_bytes);
    }

private:
    bytes m_bytes;
};

struct SubState {
    std::set<Address> suicides;  ///< Any accounts that have suicided.
    //LogEntries logs;             ///< Any logs.
    int64_t refunds = 0;         ///< Refund counter for storage changes.

    SubState& operator+=(SubState const& _s)
    {
        suicides += _s.suicides;
        refunds += _s.refunds;
        //logs += _s.logs;
        return *this;
    }

    void clear()
    {
        suicides.clear();
        //logs.clear();
        refunds = 0;
    }
};

class VM;
class VMInterface;

using OnOpFunc = std::function<void(uint64_t /*steps*/,
                                    uint64_t /* PC */,
                                    Instruction /*instr*/,
                                    bigint /*newMemSize*/,
                                    bigint /*gasCost*/,
                                    bigint /*gas*/,
                                    VM const*,
                                    VMInterface const*)>;

// struct CallParameters {
struct Arguments {
    Arguments() = default;
    Arguments(
        Address _senderAddress,
        Address _codeAddress,
        Address _receiveAddress,
        u256 _valueTransfer,
        u256 _apparentValue,
        u256 _gas,
        bytesConstRef _data,
        OnOpFunc _onOpFunc
    ):    senderAddress(_senderAddress), codeAddress(_codeAddress), receiveAddress(_receiveAddress),
        valueTransfer(_valueTransfer), apparentValue(_apparentValue), gas(_gas), data(_data), onOp(_onOpFunc)  {}
    Address senderAddress;
    Address codeAddress;
    Address receiveAddress;
    u256 valueTransfer;
    u256 apparentValue;
    u256 gas;
    bytesConstRef data;
    bool staticCall = false;
    OnOpFunc onOp;
};

class EnvInfo {
public:
    EnvInfo(BlockHeader const& _current, h256 const& _lh, u256 const& _gasUsed):
        m_header(_current),
        m_lastHashes(_lh)
        /* m_gasUsed(_gasUsed) */
    {}
    // Constructor with custom gasLimit - used in some synthetic scenarios like eth_estimateGas    RPC method
    EnvInfo(BlockHeader const& _current, h256 const& _lh, u256 const& _gasUsed, u256 const& _gasLimit):
        EnvInfo(_current, _lh, _gasUsed)
    {
        //m_headerInfo.setGasLimit(_gasLimit);
    }

    BlockHeader const& header() const { return m_header;  }

    int64_t number() const { return m_header.getNumber(); }
    Address const& author() const { return m_header.getProducer(); }
    int64_t timestamp() const { return m_header.getTimestamp(); }
    //u256 const& difficulty() const { return m_headerInfo.difficulty(); }
    //u256 const& gasLimit() const { return m_headerInfo.gasLimit(); }
    h256 const& lastHashes() const { return m_lastHashes; }
    //u256 const& gasUsed() const { return m_gasUsed; }

private:
    BlockHeader m_header;
    h256 m_lastHashes;
    // u256 m_gasUsed;
};

/// Represents a call result.
struct CallResult {
    VM_STATUS_CODE status;
    bytes output;   //owning_bytes_ref output;

    CallResult(VM_STATUS_CODE status, bytes& output)
      : status{status}, output{output}
    {}
};

/// Represents a CREATE result.
struct CreateResult {
    VM_STATUS_CODE status;
    bytes output; // owning_bytes_ref output;
    h160 address;

    CreateResult(VM_STATUS_CODE status, bytes& _output, h160 const& address)
        : status{status}, output{_output}, address{address}
    {}
};

class VMInterface {
public:
    VMInterface(EnvInfo const& _envInfo, Address const& contract, Address const& caller, Address const& origin,
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
    virtual void suicide(Address) { m_sub.suicides.insert(m_contract); }

    /// Create a new (contract) account.
    virtual CreateResult create(u256, u256&, bytesConstRef, Instruction, u256, OnOpFunc const&) = 0;

    /// Make a new message call.
    virtual CallResult call(Arguments&) = 0;

    /// Hash of a block if within the last 256 blocks, or h256() otherwise.
    virtual h256 blockHash(u256 number) = 0;

    /// Get the execution environment information.
    EnvInfo const& envInfo() const { return m_envInfo; }

    /// Return the EVM gas-price schedule for this execution context.
    virtual VMSchedule const& evmSchedule() const { return DefaultSchedule; }

private:
    EnvInfo const& m_envInfo;

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