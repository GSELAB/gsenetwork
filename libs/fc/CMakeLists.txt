
# just use

SET( ECC_IMPL secp256k1 CACHE STRING "secp256k1 or openssl or mixed" )
SET( ECC_REST crypto/elliptic_impl_priv.cpp crypto/elliptic_impl_pub.cpp )

SET( fc_sources
     uint128.cpp
     real128.cpp
     variant.cpp
     exception.cpp
     variant_object.cpp
     string.cpp
     shared_ptr.cpp
     time.cpp
     utf8.cpp
     io/datastream.cpp
     io/json.cpp
     io/varint.cpp
     io/fstream.cpp
     io/console.cpp
     filesystem.cpp
     interprocess/file_mapping.cpp
     interprocess/mmap_struct.cpp
     log/log_message.cpp
     log/logger.cpp
     log/appender.cpp
     log/console_appender.cpp
     log/gelf_appender.cpp
     log/logger_config.cpp
     crypto/_digest_common.cpp
     crypto/openssl.cpp
     crypto/aes.cpp
     crypto/crc.cpp
     crypto/city.cpp
#     crypto/base32.cpp
     crypto/base36.cpp
     crypto/base58.cpp
     crypto/base64.cpp
     crypto/bigint.cpp
     crypto/hex.cpp
     crypto/sha1.cpp
     crypto/ripemd160.cpp
     crypto/sha256.cpp
     crypto/sha224.cpp
     crypto/sha512.cpp
     crypto/dh.cpp
     crypto/blowfish.cpp
     crypto/elliptic_common.cpp
     ${ECC_REST}
     crypto/elliptic_${ECC_IMPL}.cpp
     crypto/elliptic_r1.cpp
     crypto/rand.cpp
     crypto/public_key.cpp
     crypto/private_key.cpp
     crypto/signature.cpp
     #network/ip.cpp
     #network/resolve.cpp
     #network/udp_socket.cpp
     #network/url.cpp
     #network/http/http_client.cpp
     #compress/smaz.cpp
     #compress/zlib.cpp
     )

#FILE( GLOB_RECURSE fc_headers ${CMAKE_CURRENT_SOURCE_DIR} *.hpp *.h )

#list(APPEND fc_sources ${fc_headers})

ADD_LIBRARY(fc ${fc_sources})
TARGET_LINK_LIBRARIES(fc
    Secp256k1 cryptopp-static ${GSE_BOOST_LIBS} )


