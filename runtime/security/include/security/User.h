#pragma once

#include <core/Address.h>

using namespace core;

namespace runtime {
namespace security {

enum UserType {
    ExternalUser = 0x10,
    ContractUser = 0x11
};
}
}