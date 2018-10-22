#include <string>

#include <chain/Common.h>

namespace chain {

std::string pptToString(ProtocolPacketType type)
{
    switch (type) {
        case StatusPacket: return std::string("StatusPacket");

        case NewBlockHashesPacket: return std::string("NewBlockHashesPacket");

        case TransactionPacket: return std::string("TransactionPacket");

        case TransactionsPacket: return std::string("TransactionsPacket");

        case GetBlockHeadersPacket: return std::string("GetBlockHeadersPacket");

        case BlockHeadersPacket: return std::string("BlockHeadersPacket");

        case GetBlockBodiesPacket: return std::string("GetBlockBodiesPacket");

        case BlockPacket: return std::string("BlockPacket");

        case BlockBodiesPacket: return std::string("BlockBodiesPacket");

        case NewBlockPacket: return std::string("NewBlockPacket");

        case GetNodeDataPacket: return std::string("GetNodeDataPacket");

        case NodeDataPacket: return std::string("NodeDataPacket");

        case GetReceiptsPacket: return std::string("GetReceiptsPacket");

        case ReceiptsPacket: return std::string("ReceiptsPacket");

        default: return std::string("Unknown");
    }
}

bool canHandle(ProtocolPacketType type)
{
     return (type >= StatusPacket && type <= NewBlockPacket) || (type >= GetNodeDataPacket && type <= ReceiptsPacket);
}

} // namespace chain