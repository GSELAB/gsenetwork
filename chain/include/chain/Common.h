#pragma once

namespace chain {


enum ProtocolPacketType {
    StatusPacket = 0x20,
    TransactionPacket,
    BlockPacket,
    ConfirmationPacket,
    TransactionsPacket,
    BlockStatePacket,

    GetNodeDataPacket = 0x4d,
    NodeDataPacket = 0x4e,
    GetReceiptsPacket = 0x4f,
    ReceiptsPacket = 0x50,

    PacketCount
};

enum StatusPacketType {
    GetHeight   = 0x01,
    ReplyHeight,
    SyncBlocks,
    ReplyBlocks,

    UnknownSPT = 0xFF,
};

std::string pptToString(ProtocolPacketType type);

bool canHandle(ProtocolPacketType type);

}