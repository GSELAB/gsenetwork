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

#include <core/FixedHash.h>
#include <string>
#include <vector>

#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>

namespace core {

void setThreadName(std::string const& _n);

/// Set the current thread's log name.
std::string getThreadName();

#define LOG BOOST_LOG

enum Verbosity
{
    VerbositySilent = -1,
    VerbosityError = 0,
    VerbosityWarning = 1,
    VerbosityInfo = 2,
    VerbosityDebug = 3,
    VerbosityTrace = 4,
};

// Simple cout-like stream objects for accessing common log channels.
// Thread-safe
BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(g_errorLogger,
    boost::log::sources::severity_channel_logger_mt<>,
    (boost::log::keywords::severity = VerbosityError)(boost::log::keywords::channel = "E"))
#define cerror LOG(core::g_errorLogger::get())

BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(g_warnLogger,
    boost::log::sources::severity_channel_logger_mt<>,
    (boost::log::keywords::severity = VerbosityWarning)(boost::log::keywords::channel = "W"))
#define cwarn LOG(core::g_warnLogger::get())

BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(g_noteLogger,
    boost::log::sources::severity_channel_logger_mt<>,
    (boost::log::keywords::severity = VerbosityInfo)(boost::log::keywords::channel = "I"))
#define cnote LOG(core::g_noteLogger::get())

BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(g_debugLogger,
    boost::log::sources::severity_channel_logger_mt<>,
    (boost::log::keywords::severity = VerbosityDebug)(boost::log::keywords::channel = "D"))
#define cdebug LOG(core::g_debugLogger::get())

BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(g_traceLogger,
    boost::log::sources::severity_channel_logger_mt<>,
    (boost::log::keywords::severity = VerbosityTrace)(boost::log::keywords::channel = "T"))
#define ctrace LOG(core::g_traceLogger::get())

#define CERROR LOG(core::g_errorLogger::get())
#define CWARN LOG(core::g_warnLogger::get())
#define CINFO LOG(core::g_noteLogger::get())
#define CDEBUG LOG(core::g_debugLogger::get())

using Logger = boost::log::sources::severity_channel_logger<>;
inline Logger createLogger(int _severity, std::string const& _channel)
{
    return Logger(
        boost::log::keywords::severity = _severity, boost::log::keywords::channel = _channel);
}

// Adds the context string to all log messages in the scope
#define LOG_SCOPED_CONTEXT(context) \
    BOOST_LOG_SCOPED_THREAD_ATTR("Context", boost::log::attributes::constant<std::string>(context));

}

