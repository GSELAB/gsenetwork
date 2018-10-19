#pragma once

namespace chain {


enum ProtocolPacketType: uint8_t {
    StatusPacket = 0x00,
    NewBlockHashesPacket = 0x01,
    TransactionsPacket = 0x02,
    GetBlockHeadersPacket = 0x03,
    BlockHeadersPacket = 0x04,
    GetBlockBodiesPacket = 0x05,
    BlockBodiesPacket = 0x06,
    NewBlockPacket = 0x07,

    GetNodeDataPacket = 0x0d,
    NodeDataPacket = 0x0e,
    GetReceiptsPacket = 0x0f,
    ReceiptsPacket = 0x10,

    PacketCount

};

}