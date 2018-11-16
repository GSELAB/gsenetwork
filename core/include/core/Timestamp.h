/*
 * Copyright (c) 2018 GSENetwork
 *
 * This file is part of GSENetwork.
 *
 * GSENetwork is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 */

#pragma once

namespace core {

class Timestamp {
public:
    Timestamp(int64_t time) {
        m_timestamp = time;
    }

    virtual ~Timestamp() {}

    static int64_t getNow();

    int64_t getTimestamp() {
        return m_timestamp;
    }

    void setTimestamp(int64_t time) {
        m_timestamp = time;
    }

    void operator = (Timestamp &t) {
        m_timestamp = t.getTimestamp();
    }

    bool operator > (const Timestamp &t) const {
        return m_timestamp > t.getTimestamp();
    }

    bool operator >= (const Timestamp &t) const {
        return m_timestamp >= t.getTimestamp();
    }

    bool operator < (const Timestamp &t) const {
        return m_timestamp < t.getTimestamp();
    }

    bool operator <= (const Timestamp &t) const {
        return m_timestamp <= t.getTimestamp();
    }

    bool operator == (const Timestamp &t) const {
        return m_timestamp == t.getTimestamp();
    }

    bool operator == (const Timestamp &t) const {
        return m_timestamp == t.getTimestamp();
    }

    bool operator != (const Timestamp &t) const {
        return m_timestamp != t.getTimestamp();
    }

private:
    const int64_t m_timestamp;
};

} /* end of namespace */