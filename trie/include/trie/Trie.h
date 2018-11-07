#pragma once

#include <crypto/Common.h>
#include <core/FixedHash.h>
#include <trie/TrieBase.h>

namespace trie {

extern H256 EmptyTrie;

H256 makeTrieLeft(H256 const& value);
H256 makeTrieRight(H256 const& value);

bool isTrieLeft(H256 const& value);
bool isTrieRight(H256 const& value);

inline std::pair<H256, H256> makeTriePair(H256 const& l, H256 const& r)
{
    return std::make_pair(makeTrieLeft(l), makeTrieRight(r));
}

H256 merkle(std::vector<H256> hashes);

// ------------------------------------------------------------------------------------


bytes trieRLP256(BytesMap const& bytesMap);
H256 trieHash256(BytesMap const& bytesMap);

H256 trieRoot(std::vector<bytes> const& data);
H256 trieRoot(std::vector<bytesConstRef> const& data);

}
