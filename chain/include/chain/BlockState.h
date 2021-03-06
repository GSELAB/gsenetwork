#pragma once

#include <core/Block.h>
#include <core/Object.h>
#include <producer/Schedule.h>

using namespace core;
using namespace producer;

namespace chain {

#define HEADER_CONFIRMATION_FIELDS_WITHOUT_SIG (5)
#define HEADER_CONFIRMATION_FIELDS_ALL (HEADER_CONFIRMATION_FIELDS_WITHOUT_SIG + 1)

class HeaderConfirmation: public core::Object {
public:
    HeaderConfirmation(ChainID chainID, uint64_t number, core::BlockID blockID, int64_t timestamp, core::Address producer):
        m_chainID(chainID), m_number(number), m_blockID(blockID), m_timestamp(timestamp), m_producer(producer) {}

    HeaderConfirmation(core::bytesConstRef data);

    HeaderConfirmation(HeaderConfirmation const& confirmation):
        m_chainID(confirmation.getChainID()), m_number(confirmation.getNumber()),
        m_blockID(confirmation.getBlockID()), m_timestamp(confirmation.getTimestamp()), m_producer(confirmation.getProducer()), m_signature(confirmation.getSignature()) {}

    HeaderConfirmation& operator=(HeaderConfirmation const& confirmation);

    bool operator==(HeaderConfirmation const& confirmation) const;

    bool operator!=(HeaderConfirmation const& confirmation) const;

    void streamRLP(core::RLPStream& rlpStream) const;

    void streamRLPContent(core::RLPStream& rlpStream) const;

    void sign(Secret const& privKey);

    ChainID getChainID() const { return m_chainID; }

    uint64_t getNumber() const { return m_number; }

    core::BlockID const& getBlockID() const { return m_blockID; }

    int64_t getTimestamp() const { return m_timestamp; }

    core::Address const& getProducer() const { return m_producer; }

    crypto::SignatureStruct const& getSignature() const { return m_signature; }

    core::h256 getHash();

    virtual bytes getKey() override;

    virtual bytes getRLPData() override;

    virtual Object::ObjectType getObjectType() const override { return Object::HeaderConfirmationType; }

private:
    ChainID m_chainID;
    uint64_t m_number;
    core::BlockID m_blockID;
    int64_t m_timestamp;
    core::Address m_producer;
    crypto::SignatureStruct m_signature;
};

class BlockState: public Object {
public:
    BlockState() {}

    BlockState(BlockState const& bs);

    BlockState(bytes const& bs);

    BlockState(core::Block& block);

    BlockState(bytesConstRef data);

    BlockState& operator=(BlockState const& bs);

    bool operator==(BlockState const& bs) const;

    bool operator!=(BlockState const& bs) const;

    BlockID getPrev() const { return m_block.getBlockHeader().getParentHash(); }

    void addConfirmation(HeaderConfirmation const& confirmation);

    void setProducerSnapshot(ProducerSnapshot const& ps) { m_activeProucers = ps; }

    size_t getConfirmationsSize() const { return m_confirmations.size(); }

    std::vector<HeaderConfirmation> const& getConfirmations() const { return m_confirmations; }

    bool isExistInActiveProducers(Address const& address) { return m_activeProucers.isExist(address); }

    void streamRLP(RLPStream& rlpStream) const;

    bool isSolidified() const;

    virtual bytes getKey() override;

    virtual bytes getRLPData() override;

    virtual Object::ObjectType getObjectType() const override { return Object::BlockStateType; }

public:
    core::Block m_block;

    uint64_t m_blockNumber;

    BlockID m_blockID;

    uint64_t m_bftSolidifyBlockNumber = 0;

    bool m_validated = false;

    bool m_inCurrentChain = false;

    ProducerSnapshot m_activeProucers;

    uint8_t m_confirmCount;

    mutable Mutex x_confirmations;
    std::vector<HeaderConfirmation> m_confirmations;
};

using BlockStatePtr = std::shared_ptr<BlockState>;

extern BlockStatePtr EmptyBlockStatePtr;

extern BlockState EmptyBlockState;

using HeaderConfirmationPtr = std::shared_ptr<HeaderConfirmation>;

} // namespace chain