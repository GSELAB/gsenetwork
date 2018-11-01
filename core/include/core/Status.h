#pragma once

#include <core/Object.h>
#include <core/RLP.h>
#include <core/Block.h>
#include <chain/Common.h>

using namespace chain;

namespace core {

class Status: public Object {
public:
    Status() {}

    Status(StatusPacketType type): m_type(type) {}

    Status(StatusPacketType type, uint64_t start, uint64_t end): m_type(type), m_start(start), m_end(end) {}

    Status(StatusPacketType type, uint64_t number): m_type(type), m_height(number) {}

    Status(bytesConstRef data);

    Status& operator=(Status const& status);

    void streamRLP(RLPStream& rlpStream) const;

    void populate(bytesConstRef data);

    void setType(StatusPacketType type) { m_type = type; }


    void setStartEnd(uint64_t start, uint64_t end) { m_start = start; m_end = end; }

    void setHeight(uint64_t height) { m_height = height; }

    void addBlock(Block const& block) { m_blocks.push_back(block); }

    StatusPacketType getType() const { return m_type; }

    uint64_t getHeight() const { return m_height; }

    uint64_t getStart() const { return m_start; }

    uint64_t getEnd() const { return m_end; }

    Blocks const& getBlocks() const { return m_blocks; }

    virtual bytes getKey() override;

    virtual bytes getRLPData() override;

    virtual Object::ObjectType getObjectType() const override { return Object::StatusType; }

private:
    StatusPacketType m_type = UnknownSPT;

    uint64_t m_height;

    uint64_t m_start;
    uint64_t m_end;

    Blocks m_blocks;
};

using StatusPtr = std::shared_ptr<Status>;
}