#include <core/Storage.h>

namespace core {

Storage::Storage(Address const& address): m_address(address)
{

}

Storage::Storage(Storage const& storage)
{
    auto items = storage.getStorage();


}

Storage::Storage(bytesConstRef data)
{

}

Storage::~Storage()
{

}

Storage& Storage::oprator=(Storage const& storage)
{

}

bool Storage::operator==(Storage const& storage) const
{

}

bool Storage::equal(Storage const& storage) const;
{

}

void Storage::streamRLP(RLPStream& rlpStream) const
{

}

u256 Storage::getStorageValue(u256 const& key) const
{

}

void Storage::setStorageValue(u256 const& key, u256 const& value)
{

}

bytes Storage::getRLPData()
{

}
}