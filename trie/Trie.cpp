#include <trie/Trie.h>
#include <crypto/SHA3.h>

#include <core/Log.h>
using namespace core;

using namespace crypto;

namespace trie {

H256 EmptyTrie = sha3(rlp(""));

H256 makeTrieLeft(H256 const& value)
{
    H256 ret = value;
    ret.unsetFirstBit();

    return ret;
}

H256 makeTrieRight(H256 const& value)
{
    H256 ret = value;;
    ret.setFirstBit();

    return ret;
}

bool isTrieLeft(H256 const& value)
{
    return (0 == value.checkFirstBit());
}

bool isTrieRight(H256 const& value)
{
    return (0 != value.checkFirstBit());
}

H256 merkle(std::vector<H256> ids) {

    if (0 == ids.size())
        return H256();

    while (ids.size() > 1) {
        if (ids.size() % 2)
           ids.push_back(ids.back());

        for (int i = 0; i < ids.size() / 2; i++)
            ids[i] = sha3(makeTriePair(ids[2 * i], ids[(2 * i) + 1]));

    ids.resize(ids.size() / 2);
    }

    return ids.front();
}

// --------------------------------------------------------


void hash256aux(HexMap const& _s, HexMap::const_iterator _begin, HexMap::const_iterator _end, unsigned _preLen, RLPStream& _rlp);

void hash256rlp(HexMap const& _s, HexMap::const_iterator _begin, HexMap::const_iterator _end, unsigned _preLen, RLPStream& _rlp)
{
    if (_begin == _end)
        _rlp << "";    // NULL
    else if (std::next(_begin) == _end)
    {
        // only one left - terminate with the pair.
        _rlp.appendList(2) << hexPrefixEncode(_begin->first, true, _preLen) << _begin->second;
    }
    else
    {
        // find the number of common prefix nibbles shared
        // i.e. the minimum number of nibbles shared at the beginning between the first hex string and each successive.
        unsigned sharedPre = (unsigned)-1;
        unsigned c = 0;
        for (auto i = std::next(_begin); i != _end && sharedPre; ++i, ++c)
        {
            unsigned x = std::min(sharedPre, std::min((unsigned)_begin->first.size(), (unsigned)i->first.size()));
            unsigned shared = _preLen;
            for (; shared < x && _begin->first[shared] == i->first[shared]; ++shared) {}
            sharedPre = std::min(shared, sharedPre);
        }
        if (sharedPre > _preLen)
        {
            // if they all have the same next nibble, we also want a pair.
            _rlp.appendList(2) << hexPrefixEncode(_begin->first, false, _preLen, (int)sharedPre);
            hash256aux(_s, _begin, _end, (unsigned)sharedPre, _rlp);
        }
        else
        {
            // otherwise enumerate all 16+1 entries.
            _rlp.appendList(17);
            auto b = _begin;
            if (_preLen == b->first.size())
                ++b;
            for (auto i = 0; i < 16; ++i)
            {
                auto n = b;
                for (; n != _end && n->first[_preLen] == i; ++n) {}
                if (b == n)
                    _rlp << "";
                else
                    hash256aux(_s, b, n, _preLen + 1, _rlp);
                b = n;
            }
            if (_preLen == _begin->first.size())
                _rlp << _begin->second;
            else
                _rlp << "";

        }
    }
}

void hash256aux(HexMap const& _s, HexMap::const_iterator _begin, HexMap::const_iterator _end, unsigned _preLen, RLPStream& _rlp)
{
    RLPStream rlp;
    hash256rlp(_s, _begin, _end, _preLen, rlp);
    if (rlp.out().size() < 32)
    {
        // RECURSIVE RLP
        _rlp.appendRaw(rlp.out());
    }
    else
        _rlp << sha3(rlp.out());
}

// @export
bytes trieRLP256(BytesMap const& bytesMap)
{
    if (bytesMap.empty())
        return rlp("");

    HexMap hexMap;
    for (auto i = bytesMap.rbegin(); i != bytesMap.rend(); ++i)
        hexMap[asNibbles(bytesConstRef(&i->first))] = i->second;
    RLPStream s;
    hash256rlp(hexMap, hexMap.cbegin(), hexMap.cend(), 0, s);
    return s.out();
}

// @export
H256 trieHash256(BytesMap const& bytesMap)
{
    return sha3(trieRLP256(bytesMap));
}

// @export
H256 trieRoot(std::vector<bytes> const& data)
{
    BytesMap bytesMap;
    unsigned i = 0;
    for (auto item : data) {
        bytesMap[rlp(i++)] = item;
    }

    return trieHash256(bytesMap);
}

// @export
H256 trieRoot(std::vector<bytesConstRef> const& data)
{
    BytesMap bytesMap;
    unsigned i = 0;
    for (auto item : data) {
        bytesMap[rlp(i++)] = item.toBytes();
    }

    return trieHash256(bytesMap);
}

} // namespace trie
