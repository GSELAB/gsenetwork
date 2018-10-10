#pragma once

#include <core/Block.h>

using namespace core;

namespace chain {

#define HeaderConfirmationFields 5

class HeaderConfirmation: public core::Object {
public:
    HeaderConfirmation(ChainID chainID, uint64_t number, core::BlockID blockID, core::Address producer):
        m_chainID(chainID), m_number(number), m_blockID(blockID), m_producer(producer) {}

    HeaderConfirmation(core::bytesConstRef data);

    HeaderConfirmation(HeaderConfirmation const& confirmation):
        m_chainID(confirmation.getChainID()), m_number(confirmation.getNumber()),
        m_blockID(confirmation.getBlockID()), m_producer(confirmation.getProducer()), m_signature(confirmation.getSignature()) {}

    HeaderConfirmation& operator=(HeaderConfirmation const& confirmation);

    bool operator==(HeaderConfirmation const& confirmation) const;

    bool operator!=(HeaderConfirmation const& confirmation) const;

    void streamRLP(core::RLPStream& rlpStream) const;

    void sign(Secret const& privKey);

    ChainID getChainID() const { return m_chainID; }

    uint64_t getNumber() const { return m_number; }

    core::BlockID const& getBlockID() const { return m_blockID; }

    core::Address const& getProducer() const { return m_producer; }

    crypto::SignatureStruct const getSignature() const { return m_signature; }

    core::h256 getHash();

    // @override
    bytes getKey();

    // @override
    bytes getRLPData();

    // @override
    Object::ObjectType getObjectType() const { return Object::HeaderConfirmationType; }

private:
    ChainID m_chainID;
    uint64_t m_number;
    core::BlockID m_blockID;
    core::Address m_producer;
    crypto::SignatureStruct m_signature;

    bool m_hasSigned = false;
};

class BlockState {
public:
    BlockState(core::Block const& block);

    void addConfirmation(HeaderConfirmation const& confirmation);

private:
    core::Block m_block;

    std::vector<uint8_t> m_confirmCount;
    std::vector<HeaderConfirmation> m_confirmations;


};

using BlockStatePtr = std::shared_ptr<BlockState>;

} // namespace chain