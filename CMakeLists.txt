cmake_minimum_required(VERSION 3.20)
project(nkn_sdk_cpp_priv C CXX)

set(BUILD_SHARED_LIBS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options("-g" "-O0")

find_package(Boost REQUIRED)
find_package(OpenSSL 1.1.0 REQUIRED)
find_package(Protobuf REQUIRED)

if (GMP_INCLUDE_DIR AND GMP_LIBRARY_DIR )
    set(GMP_FOUND TRUE)
else()
    find_path(GMP_INCLUDE_DIR
        NAMES gmp.h
        PATHS
        /usr/include
        /usr/local/include
        /opt/homebrew/include
        )
    find_path(GMP_LIBRARY_DIR
        NAMES "libgmp.so" "libgmp.dylib"
        PATHS
        /usr/lib/
        /usr/local/lib/
        /opt/homebrew/lib
        )
    find_library(GMP_LIBRARIES NAMES gmp
        PATHS ENV GMP_LIBRARY_DIR
        DOC "Path to the GMP library"
        )
endif()

add_subdirectory(thirdparty/cpprestsdk)

execute_process(
        COMMAND git checkout ./CMakeLists.txt ./websocketpp/common/md5.hpp
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirdparty/cpprestsdk/Release/libs/websocketpp
)

execute_process(
        COMMAND git apply ${PROJECT_SOURCE_DIR}/nkn-sdk-cpp-websocketpp.patch
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

if (APPLE)
        add_custom_target(build_libsodium ALL
                COMMAND ${PROJECT_SOURCE_DIR}/thirdparty/libsodium/autogen.sh -s
                COMMAND ${PROJECT_SOURCE_DIR}/thirdparty/libsodium/dist-build/macos.sh
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirdparty/libsodium/
                COMMENT "Building libsodium ..."
)
elseif (UNIX AND NOT APLLE)
        add_custom_target(build_libsodium ALL
                COMMAND ${PROJECT_SOURCE_DIR}/thirdparty/libsodium/autogen.sh -s
                COMMAND ${PROJECT_SOURCE_DIR}/thirdparty/libsodium/configure --prefix=${PROJECT_SOURCE_DIR}/thirdparty/libsodium/libsodium-linux
                COMMAND make && make install
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirdparty/libsodium/
                COMMENT "Building libsodium ..."
)
endif()

add_custom_target(genrate_protobuf ALL
        COMMAND protoc --proto_path=${PROJECT_SOURCE_DIR} --cpp_out=${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/pb/block.proto
        COMMAND protoc --proto_path=${PROJECT_SOURCE_DIR} --cpp_out=${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/pb/clientmessage.proto
        COMMAND protoc --proto_path=${PROJECT_SOURCE_DIR} --cpp_out=${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/pb/node.proto
        COMMAND protoc --proto_path=${PROJECT_SOURCE_DIR} --cpp_out=${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/pb/nodemessage.proto
        COMMAND protoc --proto_path=${PROJECT_SOURCE_DIR} --cpp_out=${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/pb/sigchain.proto
        COMMAND protoc --proto_path=${PROJECT_SOURCE_DIR} --cpp_out=${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/pb/transaction.proto
        COMMAND protoc --proto_path=${PROJECT_SOURCE_DIR}/payloads --cpp_out=${PROJECT_SOURCE_DIR}/payloads gogo.proto
        COMMAND protoc --proto_path=${PROJECT_SOURCE_DIR}/payloads --cpp_out=${PROJECT_SOURCE_DIR}/payloads payloads.proto
        COMMAND protoc --proto_path=${PROJECT_SOURCE_DIR}/ncp/pb --cpp_out=${PROJECT_SOURCE_DIR}/ncp/pb packet.proto
        COMMAND protoc --proto_path=${PROJECT_SOURCE_DIR}/tuna/pb --cpp_out=${PROJECT_SOURCE_DIR}/tuna/pb session.proto
        COMMENT "Generating protobuf messages ..."
)

include_directories(client)
include_directories(include)
include_directories(include/crypto)
include_directories(json)
include_directories(ncp)
include_directories(ncp/pb)
include_directories(payloads)
include_directories(pb)
include_directories(transaction)
include_directories(tuna)
include_directories(wallet)
include_directories(thirdparty/cpprestsdk/Release/include)
include_directories(thirdparty/rapidjson/include)
include_directories(thirdparty/object_threadsafe)
include_directories(thirdparty/spdlog/include)
if (APPLE)
        include_directories(thirdparty/libsodium/libsodium-osx/include)
elseif (UNIX AND NOT APPLE)
	include_directories(thirdparty/libsodium/libsodium-linux/include)
endif()
include_directories(${Boost_INCLUDE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${Protobuf_INCLUDE_DIR})
include_directories(${GMP_INCLUDE_DIR})
include_directories(.)

set(HEADERS
        client/address.h
        client/client.h
        client/config.h
        client/message.h
        client/multiclient.h
        include/crypto/aes.h
        include/crypto/base58.h
        include/crypto/crypto.h
        include/crypto/ed25519.h
        include/crypto/hash.h
        include/crypto/hex.h
        include/crypto/random.h
        include/crypto/scrypt.h
        include/crypto/secretbox.h
        include/crypto/sodium_random.h
        include/crypto/xchacha20-poly1305.h
        include/account.h
        include/byteslice.h
        include/channel.h
        include/rpc.h
        include/serialize.h
        include/SFINAE.h
        include/transaction.h
        include/uBigInt.h
        include/unique_ptr_backporting.h
        include/wallet.h
        include/walletData.h
        json/NKNCodec.h
        tuna/interface.h
        tuna/message.h
        json/NKNCodec.h
        ncp/pb/packet.pb.h
        ncp/config.h
        ncp/connection.h
        ncp/error.h
        ncp/session.h
        ncp/util.h
        payloads/gogo.pb.h
        payloads/payloads.pb.h
        pb/block.pb.h
        pb/clientmessage.pb.h
        pb/node.pb.h
        pb/nodemessage.pb.h
        pb/sigchain.pb.cc
        pb/sigchain.pb.h
        pb/transaction.pb.h
        transaction/sigchain.h
        )

set(WALLET_SOURCES
        transaction/transaction.cpp
        transaction/txBuilder.cpp
        wallet/account.cpp
        wallet/nanopay.cpp
        wallet/wallet.cpp
        wallet/walletData.cpp
        config.cpp
        rpc.cpp
        json/NKNCodec.cpp
        payloads/gogo.pb.cc
        payloads/payloads.pb.cc
        pb/block.pb.cc
        pb/clientmessage.pb.cc
        pb/node.pb.cc
        pb/nodemessage.pb.cc
        pb/sigchain.pb.cc
        pb/transaction.pb.cc
        )

set(SDK_SOURCES
        tuna/config.cpp
        tuna/client.cpp
        tuna/pb/session.pb.cc
        ncp/config.cpp
        ncp/session.cpp
        ncp/connection.cpp
        ncp/pb/packet.pb.cc
        client/config.cpp
        client/multiclient.cpp
        client/client.cpp
        client/message.cpp
        )

if (APPLE)
        set(SODIUM_LIBRARY_DIR thirdparty/libsodium/libsodium-osx/lib)
elseif (UNIX AND NOT APPLE)
	set(SODIUM_LIBRARY_DIR thirdparty/libsodium/libsodium-linux/lib)
endif()

link_directories(
        ${Boost_LIBRARIES_DIR}
        ${GMP_LIBRARY_DIR}
        ${SODIUM_LIBRARY_DIR}
        ${Protobuf_LIBRARIES_DIR}
        /opt/homebrew/lib
        ./
        )

add_library(nkn_wallet SHARED ${WALLET_SOURCES})
target_link_libraries(nkn_wallet
        -lprotobuf
        -lsodium
        -lgmp
        -lgmpxx
        ${OPENSSL_CRYPTO_LIBRARIES}
        cpprest
        )

add_library(nkn_sdk SHARED ${SDK_SOURCES})
target_link_libraries(nkn_sdk
        -lprotobuf
        -lsodium
        -lgmp
        -lgmpxx
        ${OPENSSL_CRYPTO_LIBRARIES}
        nkn_wallet
        cpprest
        )

add_executable(rpc_demo example/rpc_demo.cpp)
target_link_libraries(rpc_demo nkn_sdk)

add_executable(nkn_client_demo example/nkn_client_demo.cpp)
target_link_libraries(nkn_client_demo nkn_sdk)
