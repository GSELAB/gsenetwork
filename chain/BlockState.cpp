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
            m_timestamp = rlp[3].toInt<int64_t>();
            m_producer = rlp[4].toHash<Address>(RLP::VeryStrict);
            Signature sig = rlp[5].toHash<Signature>(RLP::VeryStrict);
            m_signature = *(SignatureStruct*)&sig;
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
    return *this;
}

bool HeaderConfirmation::operator==(HeaderConfirmation const& confirmation) const
{
    return (m_chainID == confirmation.getChainID()) &&
            (m_number == confirmation.getNumber()) &&
            (m_blockID == confirmation.getBlockID()) &&
            (m_timestamp == confirmation.getTimestamp()) &&
            (m_producer == confirmation.getProducer());
}

bool HeaderConfirmation::operator!=(HeaderConfirmation const& confirmation) const
{
    return !operator==(confirmation);
}

void HeaderConfirmation::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(HEADER_CONFIRMATION_FIELDS_ALL);
    rlpStream << (bigint)m_chainID
              << (bigint)m_number
              << m_blockID
              << (bigint)m_timestamp
              << m_producer;

    rlpStream << *(Signature*)&m_signature;
}

void HeaderConfirmation::streamRLPContent(core::RLPStream& rlpStream) const
{
    rlpStream.appendList(HEADER_CONFIRMATION_FIELDS_WITHOUT_SIG);
    rlpStream << (bigint)m_chainID
              << (bigint)m_number
              << m_blockID
              << (bigint)m_timestamp
              << m_producer;
}

void HeaderConfirmation::sign(Secret const& privKey)
{
    Signature sig = crypto::sign(privKey, getHash());
    SignatureStruct _sig = *(SignatureStruct*)&sig;
    if (_sig.isValid()) {
        m_signature = _sig;
    }
}

h256 HeaderConfirmation::getHash()
{
    RLPStream rlpStream;
    streamRLPContent(rlpStream);
    return sha3(rlpStream.out());
}

bytes HeaderConfirmation::getKey()
{
    return getHash().asBytes();
}

bytes HeaderConfirmation::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

BlockState::BlockState(BlockState const& bs)
{
    m_block = bs.m_block;
    m_blockNumber = bs.m_blockNumber;
    m_blockID = bs.m_blockID;
    m_bftSolidifyBlockNumber = bs.m_bftSolidifyBlockNumber;
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
    m_bftSolidifyBlockNumber = bs.m_bftSolidifyBlockNumber;
    m_validated = bs.m_validated;
    m_inCurrentChain = bs.m_inCurrentChain;
    m_activeProucers = bs.m_activeProucers;
    m_confirmCount = bs.m_confirmCount;
    for (auto confirmation : bs.m_confirmations)
        m_confirmations.push_back(confirmation);
    return *this;
}

bool BlockState::operator==(BlockState const& bs) const
{
    return m_blockNumber == bs.m_blockNumber &&
            m_blockID == bs.m_blockID &&
            m_activeProucers.size() == bs.m_activeProucers.size() &&
            m_confirmCount == bs.m_confirmCount;
}

bool BlockState::operator!=(BlockState const& bs) const
{
    return !operator==(bs);
}

void BlockState::addConfirmation(HeaderConfirmation const& confirmation)
{
    Guard l{x_confirmations};
    if (confirmation.getNumber() != m_blockNumber || confirmation.getBlockID() != m_blockID)
        return;

    if (!m_activeProucers.isExist(confirmation.getProducer()))
        return;

    auto itr = std::find(m_confirmations.begin(), m_confirmations.end(), confirmation);
    if (itr == m_confirmations.end()) {
        m_confirmCount++;
        m_confirmations.push_back(confirmation);
    }
}

void BlockState::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(4 + m_confirmations.size());
    rlpStream << (bigint)m_blockNumber
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

bool BlockState::isSolidified() const
{
    Guard l{x_confirmations};
    if (m_activeProucers.size() == 0) return false;
    return m_confirmCount >= ((m_activeProucers.size() * 2) / 3);
}

bytes BlockState::getKey()
{
    return m_block.getKey();
}

bytes BlockState::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}
}

