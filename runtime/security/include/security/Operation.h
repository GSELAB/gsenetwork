#pragma once

#include <security/User.h>

namespace runtime {
namespace security {

enum OperationCode {
    ReadCode    = 0x10,
    WriteCode   = 0x11,

    ContractCallCode    = 0x20,
    RecursiveCallCode   = 0x21,


};

struct Operation {
    UserType m_userType;

    OperationCode m_operationCode;
};
}
}