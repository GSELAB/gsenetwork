#pragma once

namespace chain {


enum ProtocolPacketType {
    StatusPacket = 0x20,
    NewBlockHashesPacket,
    TransactionPacket,
    BlockPacket,
    ConfirmationPacket,
    TransactionsPacket,
    GetBlockHeadersPacket,
    BlockHeadersPacket,
    GetBlockBodiesPacket,

    BlockBodiesPacket,
    NewBlockPacket,

    GetNodeDataPacket = 0x4d,
    NodeDataPacket = 0x4e,
    GetReceiptsPacket = 0x4f,
    ReceiptsPacket = 0x50,

    PacketCount
};

std::string pptToString(ProtocolPacketType type);

bool canHandle(ProtocolPacketType type);

}