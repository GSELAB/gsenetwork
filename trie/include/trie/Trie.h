#pragma once

#include <crypto/Common.h>
#include <core/RLP.h>

namespace trie {

using TrieType = core::h256;

extern TrieType EmptyTrie;

TrieType makeTrieLeft(TrieType const& value);
TrieType makeTrieRight(TrieType const& value);

bool isTrieLeft(TrieType const& value);
bool isTrieRight(TrieType const& value);

inline std::pair<TrieType, TrieType> makeTriePair(TrieType const& l, TrieType const& r)
{
    return std::make_pair(makeTrieLeft(l), makeTrieRight(r));
}

}