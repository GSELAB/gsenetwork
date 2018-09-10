#include <net/UDPx.h>
#include <crypto/Common.h>
#include <crypto/SHA3.h>

namespace net {
/* --------------------------------------------------------------- */
h256 BytesDatagramFace::sign(Secret const& sec)
{
    h256 sighash;

    return sighash;
}

Public BytesDatagramFace::authenticate(bytesConstRef sig, bytesConstRef rlp)
{
    Signature const& signature = *(Signature const*)sig.data();
    return crypto::recover(signature, sha3(rlp));
}
} // end namespace