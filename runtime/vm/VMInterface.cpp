#include <vm/VMInterface.h>

namespace runtime {
namespace vm {

VMInterface::VMInterface(EnvInfo const& envInfo, Address const& contract, Address const& caller, Address const& origin,
        uint64_t value, bytesConstRef data, bytes const& code, h256 const& codeHash, unsigned depth, bool isCreation, bool isStaticCall)
:m_envInfo(envInfo), m_contract(contract), m_caller(caller), m_origin(origin),
m_value(value), m_data(data), m_code(code), m_codeHash(codeHash), m_depth(depth), m_isCreation(isCreation), m_staticCall(isStaticCall)
{

}
}
}