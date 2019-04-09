#include <core/Storage.h>

namespace core {

Storage::Storage(Address const& address): m_address(address)
{

}

Storage::Storage(Storage const& storage)
{
    auto& items = storage.getStorages();
    m_storages.clear();
    for (auto& item : items) {
        m_storages[item.first] = item.second;
    }
}

Storage::Storage(bytesConstRef data)
{
    try {
        RLP rlp(data);

    } catch (Exception e) {
        BOOST_THROW_EXCEPTION(e);
    }

}

Storage::~Storage()
{

}

Storage& Storage::operator=(Storage const& storage)
{
    if (&storage == this) return *this;
    m_address = storage.getAddress();
    m_storages.clear();
    auto& items = storage.getStorages();
    for (auto& item : items) {
        m_storages[item.first] = item.second;
    }
    return *this;
}

bool Storage::operator==(Storage const& storage) const
{
    return m_address == storage.getAddress();
}

bool Storage::equal(Storage const& storage) const
{
    return operator==(storage);
}

void Storage::streamRLP(RLPStream& rlpStream) const
{

}

u256 Storage::getStorageValue(u256 const& key) const
{
    auto itr = m_storages.find(key);
    if (itr != m_storages.end()) {
        return itr->second;
    }

    return 0;
}

void Storage::setStorageValue(u256 const& key, u256 const& value)
{
    m_storages[key] = value;
}

bytes Storage::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}
}