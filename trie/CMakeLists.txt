SET(trie_source
    Trie.cpp)

ADD_LIBRARY(trie ${trie_source})
TARGET_LINK_LIBRARIES(trie
    utils crypto core)