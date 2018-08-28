/*
 * Copyright (c) 2018 GSENetwork
 *
 * This file is part of GSENetwork.
 *
 * GSENetwork is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 */

#include <net/BytesFrameCoder.h>
#include <net/BytesHandshake.h>
#include <net/BytesPacket.h>

using namespace std;
using namespace core;

using namespace net;

BytesFrameInfo::BytesFrameInfo(bytesConstRef frameHeader) :
    length(frameHeader[0] * 256 + frameHeader[1] * 256 + frameHeader[2]),
    padding((16 - (length % 16)) % 16),
    data(frameHeader.cropped(3).toBytes()),
    header(core::RLP(data, core::RLP::ThrowOnFail | core::RLP::FailIfTooSmall)),
    protocolId(header[0].toInt<uint16_t>()),
    multiFrame(header.itemCount() > 1),
    sequenceId(multiFrame ? header[1].toInt<uint16_t>() : 0),
    totalLength(header.itemCount() == 3 ? header[2].toInt<uint32_t>() : 0)
{

}

namespace net {

class BytesFrameCoderImpl {
public:

private:
    Mutex x_macEnc;
};
} // end namespace

BytesFrameCoder::~BytesFrameCoder() {}

BytesFrameCoder::BytesFrameCoder(BytesHandshake const&init) : m_impl(new BytesFrameCoderImpl)
{
    setup(init.m_originated, init.m_ecdheRemote, init.m_remoteNonce, init.m_ecdheLocal,
        init.m_nonce, &init.m_ackCipher, &init.m_authCipher);
}

BytesFrameCoder::BytesFrameCoder(bool originated, h512 const& remoteEphemeral, h256 const& remoteNonce, KeyPair const& ecdheLocal,
    h256 const& nonce, bytesConstRef ackCipher, bytesConstRef authCipher)
{
    setup(originated, remoteEphemeral, remoteNonce, ecdheLocal, nonce, ackCipher, authCipher);
}

void BytesFrameCoder::setup(bool originated, h512 const& remoteEphemeral, h256 const& remoteNonce, KeyPair const& ecdheLocal,
    h256 const& nonce, bytesConstRef ackClipher, bytesConstRef authCipher)
{

}

void BytesFrameCoder::writeFrame(uint16_t protocolType, bytesConstRef payload, bytes& o_bytes)
{
    core::RLPStream header;
    uint32_t len = payload.size();
    header.appendRaw(bytes({byte((len >> 16) & 0xFF), byte((len >> 8) & 0xFF), byte(len & 0xFF)}));
    header.appendList(1) << protocolType;
    writeFrame(header, payload, o_bytes);
}

void BytesFrameCoder::writeFrame(uint16_t protocolType, uint16_t seqId, bytesConstRef payload, bytes& o_bytes)
{
    core::RLPStream header;
    uint32_t len = payload.size();
    header.appendRaw(bytes({byte((len >> 16) & 0xFF), byte((len >> 8) & 0xFF), byte(len & 0xFF)}));
    header.appendList(2) << protocolType << seqId;
    writeFrame(header, payload, o_bytes);
}

void BytesFrameCoder::writeFrame(uint16_t protocolType, uint16_t seqId, uint32_t totalSize, bytesConstRef payload, bytes& o_bytes)
{
    core::RLPStream header;
    uint32_t len = payload.size();
    header.appendRaw(bytes({byte((len >> 16) & 0xFF), byte((len >> 8) & 0xFF), byte(len & 0xFF)}));
    header.appendList(3) << protocolType << seqId << totalSize;
    writeFrame(header, payload, o_bytes);
}

void BytesFrameCoder::writeFrame(core::RLPStream const& header, bytesConstRef payload, bytes& o_bytes)
{
    // TODO : works
    bytes headerWithMac(h256::size);

}

void BytesFrameCoder::writeSingleFramePacket(bytesConstRef packet, bytes& o_bytes)
{
    core::RLPStream header;
    uint32_t len = packet.size();
    header.appendRaw(bytes({byte((len >> 16) & 0xFF), byte((len >> 8) & 0xFF), byte(len & 0xFF)}));
    header.appendRaw(bytes({0xc2, 0x80, 0x80}));
    writeFrame(header, packet, o_bytes);
}

bool BytesFrameCoder::authAndDecryptHeader(bytesRef io)
{

    return true;
}

bool BytesFrameCoder::authAndDecryptFrame(bytesRef io)
{
    return true;
}

h128 BytesFrameCoder::egressDigest()
{
    h128 digest;
    return digest;
}

h128 BytesFrameCoder::ingressDigest()
{
    h128 digest;
    return digest;
}

void BytesFrameCoder::updateEgressMACWithHeader(bytesConstRef headerCipher)
{
    // TODO : add code
}

void BytesFrameCoder::updateEgressMACWithFrame(bytesConstRef cipher)
{
    // TODO : add content
}

void BytesFrameCoder::updateIngressMACWithHeader(bytesConstRef headerCipher)
{
    // TODO : add content
}

void BytesFrameCoder::updateIngressMACWithFrame(bytesConstRef cipher)
{
    // TODO : add content
}













