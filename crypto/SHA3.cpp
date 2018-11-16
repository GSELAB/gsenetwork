#include <crypto/SHA3.h>
#include <core/RLP.h>
#include <crypto/SHAx.h>

namespace crypto {

h256 const EmptySHA3 = sha3(bytesConstRef());
//h256 const EmptyListSHA3 = sha3(rlpList());

bool sha3(bytesConstRef input, bytesRef result) noexcept
{
    if (result.size() != 32)
        return false;
    sha3_256(result.data(), input.data(), input.size());
    return true;
}

} // end of namespace
