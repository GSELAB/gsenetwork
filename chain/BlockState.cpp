#include <chain/BlockState.h>
#include <core/Log.h>
#include <crypto/SHA3.h>
#include <crypto/Common.h>

using namespace core;
using namespace crypto;

namespace chain {

BlockStatePtr EmptyBlockStatePtr = BlockStatePtr();

HeaderConfirmation::HeaderConfirmation(bytesConstRef data)
{
    RLP rlp(data);
    try {
        m_chainID = rlp[0].toInt<ChainID>();
        m_number = rlp[1].toInt<uint64_t>();
        m_blockID = rlp[2].toHash<BlockID>(RLP::VeryStrict);
        m_producer = rlp[3].toHash<Address>(RLP::VeryStrict);
        Signature sig = rlp[4].toHash<Signature>(RLP::VeryStrict);
        m_signature = *(SignatureStruct*)&sig;
        m_hasSigned = true;
    } catch (Exception e) {
        CERROR << "Error interpret HeaderConfirmation!";
    }
}

HeaderConfirmation& HeaderConfirmation::operator=(HeaderConfirmation const& confirmation)
{
    if (&confirmation == this) return *this;
    m_chainID = confirmation.getChainID();
    m_number = confirmation.getNumber();
    m_blockID = confirmation.getBlockID();
    m_producer = confirmation.getProducer();
    m_signature = confirmation.getSignature();
    m_hasSigned = true;
    return *this;
}

bool HeaderConfirmation::operator==(HeaderConfirmation const& confirmation) const
{
    return (m_chainID == confirmation.getChainID()) && (m_number == confirmation.getNumber()) &&
        (m_blockID == confirmation.getBlockID()) && (m_producer == confirmation.getProducer()) &&
        (m_signature == confirmation.getSignature());
}

bool HeaderConfirmation::operator!=(HeaderConfirmation const& confirmation) const
{
    return !operator==(confirmation);
}

void HeaderConfirmation::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(HeaderConfirmationFields);
    rlpStream << m_chainID
              << m_number
              << m_blockID
              << m_producer;
    Signature sig = *(Signature*)&m_signature;
    rlpStream << sig;
}

void HeaderConfirmation::sign(Secret const& privKey)
{
    Signature sig = crypto::sign(privKey, getHash());
    SignatureStruct _sig = *(SignatureStruct*)&sig;
    if (_sig.isValid()) {
        m_signature = _sig;
        m_hasSigned = true;
    }

}

h256 HeaderConfirmation::getHash()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return sha3(rlpStream.out());
}

// @override
bytes HeaderConfirmation::getKey()
{
    return EmptyBytes;
}

// @override
bytes HeaderConfirmation::getRLPData()
{
    if (!m_hasSigned) CERROR << "Not signed!";
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

// #########################################################
BlockState::BlockState(core::Block& block):
    m_block(block)
{
    m_blockNumber = block.getNumber();
    m_blockID = block.getHash();
}

void BlockState::addConfirmation(HeaderConfirmation const& confirmation)
{

}


}

