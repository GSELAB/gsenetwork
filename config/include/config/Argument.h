#pragma once

namespace config {

class Argument {
public:
    bool m_producerON = false;

    bool m_rpcON = true;
};

extern Argument argInstance;
}