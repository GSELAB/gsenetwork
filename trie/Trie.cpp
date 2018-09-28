#include <trie/Trie.h>
#include <crypto/SHA3.h>

using namespace crypto;
using namespace core;

namespace trie {

TrieType EmptyTrie = sha3(rlp(""));

TrieType makeTrieLeft(TrieType const& value)
{
    TrieType ret;

    return ret;
}

TrieType makeTrieRight(TrieType const& value)
{
    TrieType ret;

    return ret;
}

bool isTrieLeft(TrieType const& value)
{
    return true;
}

bool isTrieRight(TrieType const& value)
{
    return true;
}

} // namespace trie