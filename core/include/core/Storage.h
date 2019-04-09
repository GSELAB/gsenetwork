#include <core/Object.h>
#include <core/Address.h>
#include <core/RLP.h>

namespace core {

class Storage: public Object {
public:
    Storage() = default;

    Storage(Address const& address);

    Storage(Storage const& storage);

    Storage(bytesConstRef data);

    virtual ~Storage();

    Storage& operator=(Storage const& storage);

    bool operator==(Storage const& storage) const;

    bool equal(Storage const& storage) const;

    void streamRLP(RLPStream& rlpStream) const;

    Address const& getAddress() const { return m_address; }

    u256 getStorageValue(u256 const& key) const;

    std::map<u256, u256> const& getStorages() const { return m_storages; }

    void setStorageValue(u256 const& key, u256 const& value);

    virtual bytes getRLPData();

    virtual bytes getKey() { return  m_address.asBytes(); }

    virtual ObjectType getObjectType() const { return Object::StorageType; }

private:
    Address m_address;
    std::map<u256, u256> m_storages;
};
}