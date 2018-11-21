#include <crypto/Valid.h>

namespace crypto {

bool validSignature(Transaction& transaction)
{
    Public pubKey = recover(transaction.getSignature(), transaction.getHash());
    return toAddress(pubKey) == transaction.getSender();
}

bool validSignature(Block& block)
{
    Public pubKey = recover(block.getSignature(), block.getHash());
    if (toAddress(pubKey) != block.getProducer()) {
        return false;
    }

    for (auto& i : block.getTransactions()) {
        if (!validSignature(*const_cast<Transaction*>(&i)))
            return false;
    }

    return true;
}

bool validSignature(HeaderConfirmation& headerConfirmation)
{
    Public pubKey = recover(headerConfirmation.getSignature(), headerConfirmation.getHash());
    return toAddress(pubKey) == headerConfirmation.getProducer();
}
}