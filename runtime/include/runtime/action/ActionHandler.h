#pragma once

#include <core/Common.h>

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