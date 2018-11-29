SET(core_source
    FixedHash.cpp
    CommonData.cpp
    Common.cpp
    Log.cpp
    RLP.cpp
    Address.cpp
    Status.cpp
    Account.cpp
    TransactionReceipt.cpp
    Transaction.cpp
    Block.cpp
    Producer.cpp
    SubChain.cpp
    AttributeState.cpp
    Ballot.cpp
    Task.cpp
    JsonHelper.cpp)

ADD_LIBRARY(core ${core_source})
TARGET_LINK_LIBRARIES(core
    utils crypto trie jsoncpp_lib_static ${GSE_BOOST_LIBS})