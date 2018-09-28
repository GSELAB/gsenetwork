#pragma once

#include <crypto/Common.h>
#include <core/FixedHash.h>
#include <core/RLP.h>
#include <trie/TrieBase.h>

namespace trie {

extern TrieType EmptyTrie;

TrieType makeTrieLeft(TrieType const& value);
TrieType makeTrieRight(TrieType const& value);

bool isTrieLeft(TrieType const& value);
bool isTrieRight(TrieType const& value);

inline std::pair<TrieType, TrieType> makeTriePair(TrieType const& l, TrieType const& r)
{
    return std::make_pair(makeTrieLeft(l), makeTrieRight(r));
}

// ------------------------------------------------------------------------------------


bytes trieRLP256(BytesMap const& bytesMap);
TrieType trieHash256(BytesMap const& bytesMap);

TrieType trieRoot(std::vector<bytes> const& data);
TrieType trieRoot(std::vector<bytesConstRef> const& data);

}