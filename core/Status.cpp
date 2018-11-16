#include <core/Status.h>
#include <core/Log.h>

namespace core {

Status::Status(bytesConstRef data)
{
    populate(data);
}

Status& Status::operator=(Status const& status)
{
    if (this == &status) return *this;
    m_type = status.getType();
    m_height = status.getHeight();
    m_start = status.getStart();
    m_end = status.getEnd();
    for (auto& i : status.getBlocks())
        m_blocks.push_back(i);
    return *this;
}

void Status::streamRLP(RLPStream& rlpStream) const
{
    switch (m_type) {
        case GetHeight:
            rlpStream.appendList(1);
            rlpStream << static_cast<uint8_t>(m_type);
            break;
        case ReplyHeight:
            rlpStream.appendList(2);
            rlpStream << static_cast<uint8_t>(m_type);
            rlpStream << (bigint) m_height;
            break;
        case SyncBlocks:
            rlpStream.appendList(3);
            rlpStream << static_cast<uint8_t>(m_type);
            rlpStream << (bigint) m_start;
            rlpStream << (bigint) m_end;
            break;
        case ReplyBlocks:
            rlpStream.appendList(1 + m_blocks.size());
            rlpStream << static_cast<uint8_t>(m_type);
            for (auto& i : m_blocks) {
                rlpStream.appendRaw((const_cast<Block*>(&i))->getRLPData());
            }
            break;
        default:
            CINFO << "Unknown type.";
            break;
    }
}

void Status::populate(bytesConstRef data)
{
    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() >= 1) {
            m_type = static_cast<StatusPacketType>(rlp[0].toInt<uint8_t>());
            switch (m_type) {
                case GetHeight:
                    break;
                case ReplyHeight:
                    m_height = rlp[1].toInt<uint64_t>();
                    break;
                case SyncBlocks:
                    m_start = rlp[1].toInt<uint64_t>();
                    m_end = rlp[2].toInt<uint64_t>();
                    break;
                case ReplyBlocks:
                    for (unsigned i = 1; i < rlp.itemCount(); i++) {
                        m_blocks.push_back(Block(rlp[i].data()));
                    }
                    break;
                default:
                    CINFO << "Unknown type";
                    break;
            }
        }
    } catch (...) {

    }
}

bytes Status::getKey()
{
    return bytes();
}

bytes Status::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}
}
