#pragma once

#include <core/Object.h>
#include <core/Address.h>
#include <core/RLP.h>

namespace core {

class Candidate: public Object {
public:
    Candidate() {}

    Candidate(Address address, uint64_t value):
        m_address(address), m_value(value) {}

    Candidate(Candidate const& candidate);

    Candidate(bytesConstRef data);

    void streamRLP(RLPStream& rlpStream) const;

    Candidate& operator=(Candidate const& candidate);

    bool operator==(Candidate const& candidate) const;

    bool operator!=(Candidate const& candidate) const;

    void set(Address const& address) { m_address = address; }

    void setValue(uint64_t value) { m_value = value; }

    Address const& getAddress() const { return m_address; }

    uint64_t getValue() const { return m_value; }

    virtual bytes getKey() override;

    virtual bytes getRLPData() override;

    virtual Object::ObjectType getObjectType() const override { return Object::BallotType; }

private:
    Address m_address;
    uint64_t m_value;
};

class Ballot: public Object {
public:
    Ballot() {}

    Ballot(bytesConstRef data);

    Ballot& operator=(Ballot const& ballot);

    bool operator==(Ballot const& vallot) const;

    bool operator!=(Ballot const& ballot) const;

    /* Add a vote record */
    void put(Address const& address, uint64_t value);

    void put(Candidate const& candidate);

    void streamRLP(RLPStream& rlpStream) const;

    std::vector<Candidate> const& getCandidateVector() const { return m_candidateVector; }

    size_t getCandidateSetSize() const { return m_candidateVector.size(); }

    virtual bytes getKey() override;

    virtual bytes getRLPData() override;

    virtual Object::ObjectType getObjectType() const override { return Object::BallotType; }

private:
    std::vector<Candidate> m_candidateVector;
};
}