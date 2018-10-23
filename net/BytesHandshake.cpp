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

#include <net/BytesHandshake.h>
#include <net/Host.h>
#include <net/Session.h>
#include <net/Peer.h>

#include <core/RLP.h>
#include <crypto/Encrypt.h>
#include <crypto/Decrypt.h>
#include <crypto/Secp256k1PP.h>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

using namespace std;
using namespace core;
using namespace net;
using namespace crypto;

void BytesHandshake::writeAuth()
{
    // send auth to the remote node
    //LOG(m_logger) << "p2p.connect.egress sending auth to " << m_socket->remoteEndpoint();
    CINFO << "p2p.connect.egress sending auth to " << m_socket->remoteEndpoint();
    m_auth.resize(Signature::size + h256::size + Public::size + h256::size + 1);
    bytesRef sig(&m_auth[0], Signature::size);
    bytesRef hepubk(&m_auth[Signature::size], h256::size);
    bytesRef pubk(&m_auth[Signature::size + h256::size], Public::size);
    bytesRef nonce(&m_auth[Signature::size + h256::size + Public::size], h256::size);

    // E(remote-pubk, S(ecdhe-random, ecdh-shared-secret^nonce) || H(ecdhe-random-pubk) || pubk || nonce || 0x0)
    Secret staticShared;
    crypto::ecdh::agree(m_host->m_key.getSecret(), m_remote, staticShared);
    sign(m_ecdheLocal.getSecret(), staticShared.makeInsecure() ^ m_nonce).ref().copyTo(sig);
    sha3(m_ecdheLocal.getPublic().ref(), hepubk);
    m_host->m_key.getPublic().ref().copyTo(pubk);
    m_nonce.ref().copyTo(nonce);
    m_auth[m_auth.size() - 1] = 0x0;
    encryptECIES(m_remote, &m_auth, m_authCipher);

    auto self(shared_from_this());
    ba::async_write(m_socket->ref(), ba::buffer(m_authCipher), [this, self](boost::system::error_code ec, std::size_t)
    {
        transition(ec);
    });
}

void BytesHandshake::writeAck()
{
    // send ack to the remote node
    //LOG(m_logger) << "p2p.connect.ingress sending ack to " << m_socket->remoteEndpoint();
    CINFO << "p2p.connect.ingress sending ack to " << m_socket->remoteEndpoint();
    m_ack.resize(Public::size + h256::size + 1);
    bytesRef epubk(&m_ack[0], Public::size);
    bytesRef nonce(&m_ack[Public::size], h256::size);
    m_ecdheLocal.getPublic().ref().copyTo(epubk);
    m_nonce.ref().copyTo(nonce);
    m_ack[m_ack.size() - 1] = 0x0;
    encryptECIES(m_remote, &m_ack, m_ackCipher);

    auto self(shared_from_this());
    ba::async_write(m_socket->ref(), ba::buffer(m_ackCipher), [this, self](boost::system::error_code ec, std::size_t)
    {
        transition(ec);
    });
}

void BytesHandshake::writeAckEIP8()
{
    // send EIP-8 format ack to remote node
    // LOG(m_logger) << "p2p.connect.ingress sending EIP-8 ack to " << m_socket->remoteEndpoint();
    CINFO << "p2p.connect.ingress sending EIP-8 ack to " << m_socket->remoteEndpoint();
    core::RLPStream rlp;
    rlp.appendList(3)
        << m_ecdheLocal.getPublic()
        << m_nonce
        << c_rlpxVersion;
    m_ack = rlp.out();
    int padAmount(rand()%100 + 100);
    m_ack.resize(m_ack.size() + padAmount, 0);

    bytes prefix(2);
    toBigEndian<uint16_t>(m_ack.size() + c_eciesOverhead, prefix);
    encryptECIES(m_remote, bytesConstRef(&prefix), &m_ack, m_ackCipher);
    m_ackCipher.insert(m_ackCipher.begin(), prefix.begin(), prefix.end());

    auto self(shared_from_this());
    ba::async_write(m_socket->ref(), ba::buffer(m_ackCipher), [this, self](boost::system::error_code ec, std::size_t)
    {
        transition(ec);
    });
}

//Signature const& sig, Public const& remotePubk, h256 const& remoteNonce, uint64_t remoteVersion);
void BytesHandshake::setAuthValues(Signature const& _sig, Public const& _remotePubk,
    h256 const& _remoteNonce, uint64_t _remoteVersion)
{
    _remotePubk.ref().copyTo(m_remote.ref());
    _remoteNonce.ref().copyTo(m_remoteNonce.ref());
    m_remoteVersion = _remoteVersion;
    Secret sharedSecret;
    crypto::ecdh::agree(m_host->m_key.getSecret(), _remotePubk, sharedSecret);
    m_ecdheRemote = recover(_sig, sharedSecret.makeInsecure() ^ _remoteNonce);
}

void BytesHandshake::readAuth()
{
    // receive auth from remote node
    //LOG(m_logger) << "p2p.connect.ingress receiving auth from " << m_socket->remoteEndpoint();
    CINFO << "p2p.connect.ingress receiving auth from " << m_socket->remoteEndpoint();
    m_authCipher.resize(307);
    auto self(shared_from_this());
    ba::async_read(m_socket->ref(), ba::buffer(m_authCipher, 307), [this, self](boost::system::error_code ec, std::size_t)
    {
        if (ec)
            transition(ec);
        else if (decryptECIES(m_host->m_key.getSecret(), bytesConstRef(&m_authCipher), m_auth))
        {
            bytesConstRef data(&m_auth);
            Signature sig(data.cropped(0, Signature::size));
            Public pubk(data.cropped(Signature::size + h256::size, Public::size));
            h256 nonce(data.cropped(Signature::size + h256::size + Public::size, h256::size));
            setAuthValues(sig, pubk, nonce, 4);
            transition();
        }
        else
            readAuthEIP8();
    });
}

void BytesHandshake::readAuthEIP8()
{
    // receive auth in EIP-8 format from remote node
    //assert(m_authCipher.size() == 307);
    if (m_authCipher.size() != 307) {
        LOG(m_logger) << "Invalid data size bytes EIP-8 auth from "
                          << m_socket->remoteEndpoint();
    }

    uint16_t size(m_authCipher[0]<<8 | m_authCipher[1]);
    //LOG(m_logger) << "p2p.connect.ingress receiving " << size << " bytes EIP-8 auth from "
    //              << m_socket->remoteEndpoint();
    CINFO << "p2p.connect.ingress receiving " << size << " bytes EIP-8 auth from "
          << m_socket->remoteEndpoint();
    m_authCipher.resize((size_t)size + 2);
    auto rest = ba::buffer(ba::buffer(m_authCipher) + 307);
    auto self(shared_from_this());
    ba::async_read(m_socket->ref(), rest, [this, self](boost::system::error_code ec, std::size_t)
    {
        bytesConstRef ct(&m_authCipher);
        if (ec)
            transition(ec);
        else if (decryptECIES(m_host->m_key.getSecret(), ct.cropped(0, 2), ct.cropped(2), m_auth))
        {
            core::RLP rlp(m_auth, core::RLP::ThrowOnFail | core::RLP::FailIfTooSmall);
            setAuthValues(
                rlp[0].toHash<Signature>(),
                rlp[1].toHash<Public>(),
                rlp[2].toHash<h256>(),
                rlp[3].toInt<uint64_t>()
            );
            m_nextState = AckAuthEIP8;
            transition();
        }
        else
        {
            LOG(m_logger) << "p2p.connect.ingress auth decrypt failed for "
                          << m_socket->remoteEndpoint();
            m_nextState = Error;
            transition();
        }
    });
}

void BytesHandshake::readAck()
{
    // receive ack from remote node
    // LOG(m_logger) << "p2p.connect.egress receiving ack from " << m_socket->remoteEndpoint();
    CINFO << "p2p.connect.egress receiving ack from " << m_socket->remoteEndpoint();
    m_ackCipher.resize(210);
    auto self(shared_from_this());
    ba::async_read(m_socket->ref(), ba::buffer(m_ackCipher, 210), [this, self](boost::system::error_code ec, std::size_t)
    {
        if (ec)
            transition(ec);
        else if (decryptECIES(m_host->m_key.getSecret(), bytesConstRef(&m_ackCipher), m_ack))
        {
            bytesConstRef(&m_ack).cropped(0, Public::size).copyTo(m_ecdheRemote.ref());
            bytesConstRef(&m_ack).cropped(Public::size, h256::size).copyTo(m_remoteNonce.ref());
            m_remoteVersion = 4;
            transition();
        }
        else
            readAckEIP8();
    });
}

void BytesHandshake::readAckEIP8()
{
    // receive EIP-8 format ack from remote node

    //assert(m_ackCipher.size() == 210);
    if (m_ackCipher.size() != 210) {
        LOG(m_logger) << "Invalid data size " << m_socket->remoteEndpoint();
    }

    uint16_t size(m_ackCipher[0]<<8 | m_ackCipher[1]);
    // LOG(m_logger) << "p2p.connect.egress receiving " << size << " bytes EIP-8 ack from " << m_socket->remoteEndpoint();
    CINFO << "p2p.connect.egress receiving " << size << " bytes EIP-8 ack from " << m_socket->remoteEndpoint();
    m_ackCipher.resize((size_t)size + 2);
    auto rest = ba::buffer(ba::buffer(m_ackCipher) + 210);
    auto self(shared_from_this());
    ba::async_read(m_socket->ref(), rest, [this, self](boost::system::error_code ec, std::size_t)
    {
        bytesConstRef ct(&m_ackCipher);
        if (ec)
            transition(ec);
        else if (decryptECIES(m_host->m_key.getSecret(), ct.cropped(0, 2), ct.cropped(2), m_ack))
        {
            core::RLP rlp(m_ack, core::RLP::ThrowOnFail | core::RLP::FailIfTooSmall);
            m_ecdheRemote = rlp[0].toHash<Public>();
            m_remoteNonce = rlp[1].toHash<h256>();
            m_remoteVersion = rlp[2].toInt<uint64_t>();
            transition();
        }
        else
        {
            LOG(m_logger) << "p2p.connect.egress ack decrypt failed for "
                          << m_socket->remoteEndpoint();
            m_nextState = Error;
            transition();
        }
    });
}

void BytesHandshake::start()
{
    transition();
}

void BytesHandshake::cancel()
{
    // cancel the handshake
    m_cancel = true;
    m_idleTimer.cancel();
    m_socket->close();
    m_io.reset();
}

void BytesHandshake::error()
{
    auto connected = m_socket->isConnected();
    if (connected && !m_socket->remoteEndpoint().address().is_unspecified())
        LOG(m_logger) << "Disconnecting " << m_socket->remoteEndpoint() << " (Handshake Failed)";
    else
        LOG(m_logger) << "Handshake Failed (Connection reset by peer)";

    cancel();
}

void BytesHandshake::transition(boost::system::error_code _ech)
{
    // reset timeout
    m_idleTimer.cancel();

    if (_ech || m_nextState == Error || m_cancel)
    {
        LOG(m_logger) << "Handshake Failed (I/O Error: " << _ech.message() << ")";
        return error();
    }

    auto self(shared_from_this());
    assert(m_nextState != StartSession);
    m_idleTimer.expires_from_now(c_timeout);
    m_idleTimer.async_wait([this, self](boost::system::error_code const& _ec)
    {
        if (!_ec)
        {
            if (!m_socket->remoteEndpoint().address().is_unspecified())
                LOG(m_logger) << "Disconnecting " << m_socket->remoteEndpoint()
                              << " (Handshake Timeout)";
            cancel();
        }
    });

    if (m_nextState == New)
    {
        m_nextState = AckAuth;
        if (m_originated)
            writeAuth();
        else
            readAuth();
    }
    else if (m_nextState == AckAuth)
    {
        m_nextState = WriteHello;
        if (m_originated)
            readAck();
        else
            writeAck();
    }
    else if (m_nextState == AckAuthEIP8)
    {
        m_nextState = WriteHello;
        if (m_originated)
            readAck();
        else
            writeAckEIP8();
    }
    else if (m_nextState == WriteHello)
    {
        m_nextState = ReadHello;
        LOG(m_logger) << (m_originated ? "p2p.connect.egress" : "p2p.connect.ingress")
                      << " sending capabilities handshake,  caps size:" << m_host->caps().size();

        /// This pointer will be freed if there is an error otherwise
        /// it will be passed to Host which will take ownership.
        m_io.reset(new BytesFrameCoder(*this));

        core::RLPStream s;
        s.append((unsigned)HelloPacket).appendList(5)
            << net::c_protocolVersion
            << m_host->m_clientVersion
            << m_host->caps()
            << m_host->listenPort()
            << m_host->id();

        bytes packet;
        s.swapOut(packet);
        m_io->writeSingleFramePacket(&packet, m_handshakeOutBuffer);
        ba::async_write(m_socket->ref(), ba::buffer(m_handshakeOutBuffer), [this, self](boost::system::error_code ec, std::size_t)
        {
            transition(ec);
        });
    }
    else if (m_nextState == ReadHello)
    {
        // Authenticate and decrypt initial hello frame with initial RLPXFrameCoder
        // and request m_host to start session.
        m_nextState = StartSession;

        // read frame header
        unsigned const handshakeSize = 32;
        m_handshakeInBuffer.resize(handshakeSize);
        ba::async_read(m_socket->ref(), boost::asio::buffer(m_handshakeInBuffer, handshakeSize), [this, self](boost::system::error_code ec, std::size_t)
        {
            if (ec)
                transition(ec);
            else
            {
                if (!m_io)
                {
                    LOG(m_logger) << "Internal error in handshake: RLPXFrameCoder disappeared.";
                    m_nextState = Error;
                    transition();
                    return;

                }
                /// authenticate and decrypt header
                if (!m_io->authAndDecryptHeader(bytesRef(m_handshakeInBuffer.data(), m_handshakeInBuffer.size())))
                {
                    m_nextState = Error;
                    transition();
                    return;
                }

                LOG(m_logger) << (m_originated ? "p2p.connect.egress" : "p2p.connect.ingress")
                                 << " recvd hello header";

                /// check frame size
                bytes& header = m_handshakeInBuffer;
                uint32_t frameSize = (uint32_t)(header[2]) | (uint32_t)(header[1])<<8 | (uint32_t)(header[0])<<16;
                if (frameSize > 1024)
                {
                    // all future frames: 16777216
                    LOG(m_logger)
                        << (m_originated ? "p2p.connect.egress" : "p2p.connect.ingress")
                        << " hello frame is too large " << frameSize;
                    m_nextState = Error;
                    transition();
                    return;
                }

                /// rlp of header has protocol-type, sequence-id[, total-packet-size]
                bytes headerRLP(header.size() - 3 - h128::size);	// this is always 32 - 3 - 16 = 13. wtf?
                bytesConstRef(&header).cropped(3).copyTo(&headerRLP);

                /// read padded frame and mac
                m_handshakeInBuffer.resize(frameSize + ((16 - (frameSize % 16)) % 16) + h128::size);
                ba::async_read(m_socket->ref(), boost::asio::buffer(m_handshakeInBuffer, m_handshakeInBuffer.size()), [this, self, headerRLP](boost::system::error_code ec, std::size_t)
                {
                    m_idleTimer.cancel();

                    if (ec)
                        transition(ec);
                    else
                    {
                        if (!m_io)
                        {
                            LOG(m_logger) << "Internal error in handshake: RLPXFrameCoder disappeared.";
                            m_nextState = Error;
                            transition();
                            return;

                        }
                        bytesRef frame(&m_handshakeInBuffer);
                        if (!m_io->authAndDecryptFrame(frame))
                        {
                            cnetdetails
                                << (m_originated ? "p2p.connect.egress" : "p2p.connect.ingress")
                                << " hello frame: decrypt failed";
                            m_nextState = Error;
                            transition();
                            return;
                        }

                        PacketType packetType = frame[0] == 0x80 ? HelloPacket : (PacketType)frame[0];
                        if (packetType != HelloPacket)
                        {
                            cnetdetails
                                << (m_originated ? "p2p.connect.egress" : "p2p.connect.ingress")
                                << " hello frame: invalid packet type";
                            m_nextState = Error;
                            transition();
                            return;
                        }

                        cnetdetails << (m_originated ? "p2p.connect.egress" : "p2p.connect.ingress")
                                    << " hello frame: success. starting session.";
                        try
                        {
                            core::RLP rlp(frame.cropped(1), core::RLP::ThrowOnFail | core::RLP::FailIfTooSmall);
                            m_host->startPeerSession(m_remote, rlp, move(m_io), m_socket);
                        }
                        catch (std::exception const& _e)
                        {
                            cnetlog << "Handshake causing an exception: " << _e.what();
                            m_nextState = Error;
                            transition();
                        }
                    }
                });
            }
        });
    }

}

