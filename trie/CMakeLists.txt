SET(trie_source
    TrieBase.cpp
    Trie.cpp)

ADD_LIBRARY(trie ${trie_source})
TARGET_LINK_LIBRARIES(trie
    utils crypto core)