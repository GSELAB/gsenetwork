#pragma once

#include <crypto/Common.h>
#include <core/Transaction.h>


using namespace core;

namespace crypto {

bool isValidSig(Transaction const& transaction);

}