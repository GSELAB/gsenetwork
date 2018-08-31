SET(core_source
    FixedHash.cpp
    CommonData.cpp
    Common.cpp
    Log.cpp
    RLP.cpp
    TransactionReceipt.cpp
    Transaction.cpp
    Block.cpp
    Address.cpp
    Worker.cpp)

ADD_LIBRARY(core ${core_source})
TARGET_LINK_LIBRARIES(core
    utils crypto ${GSE_BOOST_LIBS})