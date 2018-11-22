#include <core/Ballot.h>
#include <core/Log.h>
#include <core/Exceptions.h>

namespace core {

Candidate::Candidate(Candidate const& candidate)
{
    m_address = candidate.getAddress();
    m_value = candidate.getValue();
}

Candidate::Candidate(bytesConstRef data)
{
    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() == 2) {
            m_address = rlp[0].toHash<Address>(RLP::VeryStrict);
            m_value = rlp[1].toInt<uint64_t>();
        } else {
            CERROR << "Not Candidate bytes data!";
            throw DeserializeException("Not Candidate bytes data!");
        }
    } catch (DeserializeException e) {
        throw e;
    } catch (Exception e) {
        throw DeserializeException("Deserialize Candidate failed!");
    }
}

void Candidate::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(2);
    rlpStream << m_address << (bigint) m_value;
}

Candidate& Candidate::operator=(Candidate const& candidate)
{
    if (&candidate == this) return *this;
    m_address = candidate.getAddress();
    m_value = candidate.getValue();
    return *this;
}

bool Candidate::operator==(Candidate const& candidate) const
{
    return (m_address == candidate.getAddress()) && (m_value == candidate.getValue());
}

bool Candidate::operator!=(Candidate const& candidate) const
{
    return !operator==(candidate);
}

bytes Candidate::getKey()
{
    return bytes();
}

bytes Candidate::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

Ballot::Ballot(bytesConstRef data)
{
    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() > 0) {
            for (unsigned i = 0; i < rlp.itemCount(); i++) {
                bytesConstRef itemRef = rlp[i].data();
                Candidate candidate(itemRef);
                m_candidateVector.push_back(candidate);
            }
        } else {
            CERROR << "Not Ballot bytes data";
            throw DeserializeException("Not Ballot bytes data");
        }
    } catch (DeserializeException e) {
        throw e;
    } catch (Exception e) {
        CERROR << "Deserialize Ballot failed!";
        throw DeserializeException("Deserialize Ballot failed!");
    }
}

Ballot& Ballot::operator=(Ballot const& ballot)
{
    if (&ballot == this) return *this;
    for (auto const& i : ballot.getCandidateVector())
        m_candidateVector.push_back(i);

    return *this;
}

bool Ballot::operator==(Ballot const& vallot) const
{
    return m_candidateVector == vallot.getCandidateVector();
}

bool Ballot::operator!=(Ballot const& ballot) const
{
    return !operator==(ballot);
}

void Ballot::put(Address const& address, uint64_t value)
{
    Candidate candidate(address, value);
    m_candidateVector.push_back(candidate);
}

void Ballot::put(Candidate const& candidate)
{
    m_candidateVector.push_back(candidate);
}

void Ballot::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(getCandidateSetSize());
    for (auto& i : m_candidateVector) {
        Candidate candidate = i;
        rlpStream.appendRaw(candidate.getRLPData());
    }
}

bytes Ballot::getKey()
{
    return bytes();
}

bytes Ballot::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

}
