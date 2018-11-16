#include <net/UDPx.h>
#include <crypto/Common.h>
#include <crypto/SHA3.h>
#include <core/RLP.h>

using namespace core;
using namespace crypto;

namespace net {

h256 BytesDatagramFace::sign(Secret const& sec)
{
    core::RLPStream rlpStream;
    rlpStream.appendRaw(bytes(1, packetType()));
    streamRLP(rlpStream);
    bytes rlpData(rlpStream.out());

    // hash(Sig + rlpData) + signature + rlpData
    h256 hash = sha3(&rlpData);
    Signature sig = crypto::sign(sec, hash);
    data.resize(h256::size + Signature::size + rlpData.size());
    bytesRef hashRef(&data[0], h256::size);
    bytesRef sigRef(&data[h256::size], Signature::size);
    bytesRef dataRef(&data[data.size() - rlpData.size()], rlpData.size());

    sig.ref().copyTo(sigRef);
    bytesConstRef(&rlpData).copyTo(dataRef);

    bytesConstRef _hashContent(&data[h256::size], data.size() - h256::size);
    h256 _hash = sha3(_hashContent);
    _hash.ref().copyTo(hashRef);
    return hash;
}

Public BytesDatagramFace::authenticate(bytesConstRef sig, bytesConstRef rlp)
{
    Signature const& signature = *(Signature const*)sig.data();
    return crypto::recover(signature, sha3(rlp));
}
} // end namespace