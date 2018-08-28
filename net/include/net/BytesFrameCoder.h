#pragma once

#include <core/Exceptions.h>
#include <core/RLP.h>

#include <net/Common.h>

using namespace core;

namespace net {

struct BytesFrameDecryptFailed : virtual core::Exception {};


struct BytesFrameInfo {
    BytesFrameInfo() = default;
    /// Constructor. frame-size || protocol-type, [sequence-id[, total-packet-size]]
    BytesFrameInfo(bytesConstRef frameHeader);

    uint32_t const length; // size of frame (excludes padding) max:2**24
    uint8_t const padding; // length of padding which follows @lenght

    bytes const data;  // bytes of header
    core::RLP const header;  // header RLP

    uint16_t const protocolId;  // protocol ID as negotiated by handshake
    bool const multiFrame;   // if this frame is part of sequence
    uint16_t const sequenceId; // sequence id if frame
    uint32_t const totalLength; // set to total length of packet in first frame of multiframe packet.
};

class BytesHandshake;


/**
 * @brief Encoder/decoder transport for RLPx connection established by BytesHandshake.
 *
 * Thread Safety
 * Distinct Objects: Unsafe.
 * Shared objects: Unsafe.
 */
class BytesFrameCoder {
    friend class Session;
public:
    // requires instance of BytesHandshake which has encrypted ECDH key exchange
    BytesFrameCoder(BytesHandshake const& init);

    //
    BytesFrameCoder(bool originated, h512 const& remoteEphemeral, h256 const& remoteNonce, KeyPair const& ecdheLocal,
        h256 const& nonce, bytesConstRef ackClipher, bytesConstRef authCipher);

    ~BytesFrameCoder();

    // establish shared secrets and setup aes and mac states
    void setup(bool originated, h512 const& remoteEphemeral, h256 const& remoteNonce, KeyPair const& ecdheLocal,
        h256 const& nonce, bytesConstRef ackClipher, bytesConstRef authCipher);

    // write single-frame payload of packet(s)
    void writeFrame(uint16_t protocolType, bytesConstRef payload, bytes& o_bytes);

    // write continuation frame of segmented payload
    void writeFrame(uint16_t protocolType, uint16_t seqId, bytesConstRef payload, bytes& o_bytes);

    // write first frame of segmented or sequence-tagged payload
    void writeFrame(uint16_t protocolType, uint16_t seqId, uint32_t totalSize, bytesConstRef payload, bytes& o_bytes);

    // Legacy. encrypt packet as ill-defined legacy bytes frame
    void writeSingleFramePacket(bytesConstRef packet, bytes& o_bytes);

    // authenticate and decrypt header in-place.
    bool authAndDecryptHeader(bytesRef io_cipherWithMac);

    // authenticate and decrypt frame in-place
    bool authAndDecryptFrame(bytesRef io_cipherWithMac);

    // return first 16 bytes of current digest from egress mac.
    h128 egressDigest();

    // return first 16 bytes of current digest from ingress mac.
    h128 ingressDigest();

protected:
    void writeFrame(core::RLPStream const& header, bytesConstRef payload, bytes& o_bytes);

    // update state of egress mac with frame header
    void updateEgressMACWithHeader(bytesConstRef headerCipher);

    // update state if egress MAC with frame
    void updateEgressMACWithFrame(bytesConstRef cipher);

    // update state of ingress MAC with frame header.
    void updateIngressMACWithHeader(bytesConstRef headerCipher);

    // update state of ingress MAC with frame
    void updateIngressMACWithFrame(bytesConstRef cipher);

private:
    std::unique_ptr<class BytesFrameCoderImpl> m_impl;
};


} // end namespace