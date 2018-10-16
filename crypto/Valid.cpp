#include <crypto/Valid.h>

namespace crypto {
bool isValidSig(Transaction& transaction)
{
    Public pubKey = recover(transaction.getSig(), transaction.getHash());
    return verify(pubKey, transaction.getSig(), transaction.getHash());
}

}