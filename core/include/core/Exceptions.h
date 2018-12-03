#pragma once

#include "FixedHash.h"
#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception/errinfo_api_function.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/info_tuple.hpp>
#include <boost/throw_exception.hpp>
#include <boost/tuple/tuple.hpp>
#include <exception>
#include <string>

namespace core {

/// Base class for all exceptions.
struct Exception : virtual std::exception, virtual boost::exception
{
    const char* what() const noexcept override { return boost::diagnostic_information_what(*this); }
};


#define DEV_SIMPLE_EXCEPTION(X)  \
    struct X : virtual Exception \
    {                            \
    }

/// Base class for all RLP exceptions.
struct RLPException : virtual Exception
{
};
#define DEV_SIMPLE_EXCEPTION_RLP(X) \
    struct X : virtual RLPException \
    {                               \
    }

DEV_SIMPLE_EXCEPTION_RLP(BadCast);
DEV_SIMPLE_EXCEPTION_RLP(BadRLP);
DEV_SIMPLE_EXCEPTION_RLP(OversizeRLP);
DEV_SIMPLE_EXCEPTION_RLP(UndersizeRLP);

DEV_SIMPLE_EXCEPTION(BadHexCharacter);
DEV_SIMPLE_EXCEPTION(NoNetworking);
DEV_SIMPLE_EXCEPTION(NoUPnPDevice);
DEV_SIMPLE_EXCEPTION(RootNotFound);
DEV_SIMPLE_EXCEPTION(BadRoot);
DEV_SIMPLE_EXCEPTION(FileError);
DEV_SIMPLE_EXCEPTION(Overflow);
DEV_SIMPLE_EXCEPTION(FailedInvariant);
DEV_SIMPLE_EXCEPTION(ValueTooLarge);
DEV_SIMPLE_EXCEPTION(UnknownField);
DEV_SIMPLE_EXCEPTION(MissingField);
DEV_SIMPLE_EXCEPTION(WrongFieldType);
DEV_SIMPLE_EXCEPTION(InterfaceNotSupported);
DEV_SIMPLE_EXCEPTION(ExternalFunctionFailure);

// error information to be added to exceptions
using errinfo_invalidSymbol = boost::error_info<struct tag_invalidSymbol, char>;
using errinfo_wrongAddress = boost::error_info<struct tag_address, std::string>;
using errinfo_comment = boost::error_info<struct tag_comment, std::string>;
using errinfo_required = boost::error_info<struct tag_required, bigint>;
using errinfo_got = boost::error_info<struct tag_got, bigint>;
using errinfo_min = boost::error_info<struct tag_min, bigint>;
using errinfo_max = boost::error_info<struct tag_max, bigint>;
using RequirementError = boost::tuple<errinfo_required, errinfo_got>;
using RequirementErrorComment = boost::tuple<errinfo_required, errinfo_got, errinfo_comment>;
using errinfo_hash256 = boost::error_info<struct tag_hash, h256>;
using errinfo_required_h256 = boost::error_info<struct tag_required_h256, h256>;
using errinfo_got_h256 = boost::error_info<struct tag_get_h256, h256>;
using Hash256RequirementError = boost::tuple<errinfo_required_h256, errinfo_got_h256>;
using errinfo_extraData = boost::error_info<struct tag_extraData, bytes>;
using errinfo_externalFunction = boost::errinfo_api_function;
using errinfo_interface = boost::error_info<struct tag_interface, std::string>;
using errinfo_path = boost::error_info<struct tag_path, std::string>;


// information to add to exceptions
using errinfo_name = boost::error_info<struct tag_field, std::string>;
using errinfo_field = boost::error_info<struct tag_field, int>;
using errinfo_data = boost::error_info<struct tag_data, std::string>;
using errinfo_target = boost::error_info<struct tag_target, h256>;
using BadFieldError = boost::tuple<errinfo_field, errinfo_data>;


struct GSException: virtual public std::exception, virtual boost::exception {
public:
    explicit GSException(std::string const& message, int number = 0): m_errorMessage(message), m_number(number) {}

    ~GSException() throw() {}

    virtual const char* what() const noexcept override { return m_errorMessage.c_str(); }
private:
    int m_number = 0;
    std::string m_errorMessage;
};

#define THROW_GSEXCEPTION(str) \
    do { \
        throw GSException(str); \
    } while(0)

#define CORE_TEMPLATE_EXCEPTION(name) \
struct name##Exception: virtual GSException { \
    explicit name##Exception(std::string const& message, int number = 0): \
        GSException(message, number) {} \
}

#define RUNTIME_TEMPLATE_EXCEPTION(name) \
struct name##Exception: virtual GSException { \
    explicit name##Exception(std::string const& message, int number = 0): \
        GSException(message, number) {} \
}

#define CHAIN_TEMPLATE_EXCEPTION(name) \
struct name##Exception: virtual GSException { \
    explicit name##Exception(std::string const& message, int number = 0): \
        GSException(message, number) {} \
}

#define PRODUCER_TEMPLATE_EXCEPTION(name) \
struct name##Exception: virtual GSException { \
    explicit name##Exception(std::string const& message, int number = 0): \
        GSException(message, number) {} \
}

#define RPC_TEMPLATE_EXCEPTION(name) \
struct name##Exception: virtual GSException { \
    explicit name##Exception(std::string const& message, int number = 0): \
        GSException(message, number) {} \
}

#define NET_TEMPLATE_EXCEPTION(name) \
struct name##Exception: virtual GSException { \
    explicit name##Exception(std::string const& message, int number = 0): \
        GSException(message, number) {} \
}

#define PRODUCERSERVER_TEMPLATE_EXCEPTION(name) \
struct name##Exception: virtual GSException { \
    explicit name##Exception(std::string const& message, int number = 0): \
        GSException(message, number) {} \
}

CORE_TEMPLATE_EXCEPTION(Deserialize);
CORE_TEMPLATE_EXCEPTION(Serialize);
CORE_TEMPLATE_EXCEPTION(VoteNotExistProducer);
CORE_TEMPLATE_EXCEPTION(VoteNotExistAccount);
CORE_TEMPLATE_EXCEPTION(InvalidTransaction);
CORE_TEMPLATE_EXCEPTION(InvalidProducer);
CORE_TEMPLATE_EXCEPTION(AttributeState);

RUNTIME_TEMPLATE_EXCEPTION(Repository);

CHAIN_TEMPLATE_EXCEPTION(BlockChain);
CHAIN_TEMPLATE_EXCEPTION(RollbackState);
CHAIN_TEMPLATE_EXCEPTION(RollbackStateAncestor);

PRODUCER_TEMPLATE_EXCEPTION(Producer);

NET_TEMPLATE_EXCEPTION(NetController);

RPC_TEMPLATE_EXCEPTION(RPC);

PRODUCERSERVER_TEMPLATE_EXCEPTION(ProducerServer);
}
