#include <chain/BlockState.h>
#include <core/Log.h>
#include <crypto/SHA3.h>
#include <crypto/Common.h>

using namespace core;
using namespace crypto;

namespace chain {

BlockStatePtr EmptyBlockStatePtr = BlockStatePtr();
BlockState EmptyBlockState;

HeaderConfirmation::HeaderConfirmation(bytesConstRef data)
{
    RLP rlp(data);
    try {
        if (rlp.isList() && rlp.itemCount() == HEADER_CONFIRMATION_FIELDS_ALL) {
            m_chainID = rlp[0].toInt<ChainID>();
            m_number = rlp[1].toInt<uint64_t>();
            m_blockID = rlp[2].toHash<BlockID>(RLP::VeryStrict);
            m_timestamp = rlp[3].toInt<uint64_t>();
            m_producer = rlp[4].toHash<Address>(RLP::VeryStrict);
            Signature sig = rlp[5].toHash<Signature>(RLP::VeryStrict);
            m_signature = *(SignatureStruct*)&sig;
            m_hasSigned = true;
        } else {
            throw DeserializeException("Deserialize HeaderConfirmation failed!");
        }
    } catch (DeserializeException& e) {
        throw e;
    } catch (GSException& e) {
        CERROR << "Error interpret HeaderConfirmation!";
        throw e;
    }
}

HeaderConfirmation& HeaderConfirmation::operator=(HeaderConfirmation const& confirmation)
{
    if (&confirmation == this) return *this;
    m_chainID = confirmation.getChainID();
    m_number = confirmation.getNumber();
    m_blockID = confirmation.getBlockID();
    m_timestamp = confirmation.getTimestamp();
    m_producer = confirmation.getProducer();
    m_signature = confirmation.getSignature();
    m_hasSigned = true;
    return *this;
}

bool HeaderConfirmation::operator==(HeaderConfirmation const& confirmation) const
{
    return (m_chainID == confirmation.getChainID()) &&
            (m_number == confirmation.getNumber()) &&
            (m_blockID == confirmation.getBlockID()) &&
            (m_timestamp == confirmation.getTimestamp()) &&
            (m_producer == confirmation.getProducer()) ;
}

bool HeaderConfirmation::operator!=(HeaderConfirmation const& confirmation) const
{
    return !operator==(confirmation);
}

void HeaderConfirmation::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(HEADER_CONFIRMATION_FIELDS_ALL);
    rlpStream << m_chainID
              << m_number
              << m_blockID
              << m_timestamp
              << m_producer;

    rlpStream << *(Signature*)&m_signature;
}

void HeaderConfirmation::streamRLPContent(core::RLPStream& rlpStream) const
{
    rlpStream.appendList(HEADER_CONFIRMATION_FIELDS_WITHOUT_SIG);
    rlpStream << m_chainID
              << m_number
              << m_blockID
              << m_timestamp
              << m_producer;
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
    streamRLPContent(rlpStream);
    return sha3(rlpStream.out());
}

// @override
bytes HeaderConfirmation::getKey()
{
    return getHash().asBytes();
}

// @override
bytes HeaderConfirmation::getRLPData()
{
    // if (!m_hasSigned) CERROR << "Not signed!";
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

// #########################################################
BlockState::BlockState(BlockState const& bs)
{
    m_block = bs.m_block;
    m_blockNumber = bs.m_blockNumber;
    m_blockID = bs.m_blockID;
    // m_dposIrreversibleBlockNumber = bs.m_dposIrreversibleBlockNumber;
    m_bftIrreversibleBlockNumber = bs.m_bftIrreversibleBlockNumber;
    m_validated = bs.m_validated;
    m_inCurrentChain = bs.m_inCurrentChain;
    m_activeProucers = bs.m_activeProucers;
    m_confirmCount = bs.m_confirmCount;
    for (auto confirmation : bs.m_confirmations)
        m_confirmations.push_back(confirmation);
}

BlockState::BlockState(core::Block& block):
    m_block(block)
{
    m_blockNumber = block.getNumber();
    m_blockID = block.getHash();
}

BlockState::BlockState(bytesConstRef data)
{
    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() >= 4) {
            m_blockNumber = rlp[0].toInt<uint64_t>();
            m_blockID = rlp[1].toHash<BlockID>(RLP::VeryStrict);
            bytesConstRef aps = rlp[2].data();
            m_activeProucers.populate(aps);
            m_confirmCount = rlp[3].toInt<uint8_t>();
            for (unsigned i = 4; i < rlp.itemCount(); i ++) {
                bytesConstRef hcBytes = rlp[i].data();
                HeaderConfirmation hc(hcBytes);
                m_confirmations.push_back(hc);
            }
        }
    } catch (...) {

    }
}

BlockState::BlockState(bytes const& data): BlockState(bytesConstRef(&data))
{

}

BlockState& BlockState::operator=(BlockState const& bs)
{
    if (this == &bs) return *this;
    m_block = bs.m_block;
    m_blockNumber = bs.m_blockNumber;
    m_blockID = bs.m_blockID;
    // m_dposIrreversibleBlockNumber = bs.m_dposIrreversibleBlockNumber;
    m_bftIrreversibleBlockNumber = bs.m_bftIrreversibleBlockNumber;
    m_validated = bs.m_validated;
    m_inCurrentChain = bs.m_inCurrentChain;
    m_activeProucers = bs.m_activeProucers;
    m_confirmCount = bs.m_confirmCount;
    for (auto confirmation : bs.m_confirmations)
        m_confirmations.push_back(confirmation);
    return *this;
}

void BlockState::addConfirmation(HeaderConfirmation const& confirmation)
{
    if (confirmation.getNumber() != m_blockNumber || confirmation.getBlockID() != m_blockID)
        return;

    if (!m_activeProucers.isExist(confirmation.getProducer()))
        return;

    m_confirmCount++;
    m_confirmations.push_back(confirmation);
}

void BlockState::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(4 + m_confirmations.size());
    rlpStream << m_blockNumber
              << m_blockID;
    {
        RLPStream rlpPS;
        m_activeProucers.streamRLP(rlpPS);
        rlpStream.appendRaw(rlpPS.out());
    }
    rlpStream << m_confirmCount;
    for (auto i : m_confirmations) {
        rlpStream.appendRaw(i.getRLPData());
    }
}

bytes BlockState::getKey()
{
    // use block key
    return m_block.getKey();
}

bytes BlockState::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}


}

