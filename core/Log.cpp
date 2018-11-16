/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Log.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <core/Log.h>

#ifdef __APPLE__
#include <pthread.h>
#endif

#include <boost/core/null_deleter.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/function.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/exception_handler.hpp>

#if defined(NDEBUG)
#include <boost/log/sinks/async_frontend.hpp>
template <class T>
using log_sink = boost::log::sinks::asynchronous_sink<T>;
#else
#include <boost/log/sinks/sync_frontend.hpp>
template <class T>
using log_sink = boost::log::sinks::synchronous_sink<T>;
#endif

namespace core {

namespace
{
/// Associate a name with each thread for nice logging.
struct ThreadLocalLogName
{
    ThreadLocalLogName(std::string const& _name) { m_name.reset(new std::string(_name)); }
    boost::thread_specific_ptr<std::string> m_name;
};

ThreadLocalLogName g_logThreadName("main");
}  // namespace



std::string getThreadName()
{
#if defined(__GLIBC__) || defined(__APPLE__)
    char buffer[128];
    pthread_getname_np(pthread_self(), buffer, 127);
    buffer[127] = 0;
    return buffer;
#else
    return g_logThreadName.m_name.get() ? *g_logThreadName.m_name.get() : "<unknown>";
#endif
}

void setThreadName(std::string const& _n)
{
#if defined(__GLIBC__)
    pthread_setname_np(pthread_self(), _n.c_str());
#elif defined(__APPLE__)
    pthread_setname_np(_n.c_str());
#else
    g_logThreadName.m_name.reset(new std::string(_n));
#endif
}



}