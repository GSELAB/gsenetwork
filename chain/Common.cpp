#include <string>

#include <chain/Common.h>

namespace chain {

std::string pptToString(ProtocolPacketType type)
{
    switch (type) {
        case StatusPacket: return std::string("StatusPacket");
        case TransactionPacket: return std::string("TransactionPacket");
        case TransactionsPacket: return std::string("TransactionsPacket");
        case BlockPacket: return std::string("BlockPacket");
        case ConfirmationPacket: return std::string("ConfirmationPacket");
        case BlockStatePacket: return std::string("BlockStatePacket");
        case GetNodeDataPacket: return std::string("GetNodeDataPacket");
        case NodeDataPacket: return std::string("NodeDataPacket");
        case GetReceiptsPacket: return std::string("GetReceiptsPacket");
        case ReceiptsPacket: return std::string("ReceiptsPacket");
        default: return std::string("Unknown");
    }
}

bool canHandle(ProtocolPacketType type)
{
     return (type >= StatusPacket && type <= BlockStatePacket) || (type >= GetNodeDataPacket && type <= ReceiptsPacket);
}

}