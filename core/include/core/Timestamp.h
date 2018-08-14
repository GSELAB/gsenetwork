/**
 *  @author guoygang <hero.gariker@gmail.com>
 *  @date 2018
 */

#ifndef __TIMESTAMP_HEADER__
#define __TIMESTAMP_HEADER__

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
    int64_t m_timestamp;
};

} /* end of namespace */
#endif