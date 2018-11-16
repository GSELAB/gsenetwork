#include <crypto/Valid.h>

namespace crypto {

bool isValidSig(Transaction& transaction)
{
    Public pubKey = recover(transaction.getSignature(), transaction.getHash());
    return toAddress(pubKey) == transaction.getSender();
}

bool isValidSig(Block& block)
{
    Public pubKey = recover(block.getSignature(), block.getHash());
    return toAddress(pubKey) == block.getProducer();
}

bool isValidSig(HeaderConfirmation& headerConfirmation)
{
    Public pubKey = recover(headerConfirmation.getSignature(), headerConfirmation.getHash());
    return toAddress(pubKey) == headerConfirmation.getProducer();
}

}