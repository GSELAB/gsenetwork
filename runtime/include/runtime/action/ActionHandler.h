#pragma once

#include <core/Common.h>
#include <core/Block.h>
#include <config/Constant.h>

namespace runtime {
namespace action {

//@interface Action
class ActionHandler {
public:
    virtual ~ActionHandler() {}

    virtual void init() = 0;

    virtual void execute() = 0;

    virtual void finalize() = 0;
};
}
}