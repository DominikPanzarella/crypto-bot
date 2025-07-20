cmake_minimum_required(VERSION 3.20)
project(ArbitrageBot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Ottimizzazione bassa latenza
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native -flto -fno-exceptions -fno-rtti)
endif()

# Opzioni
option(BUILD_TESTING "Enable unit tests" ON)
option(BUILD_BENCHMARKS "Enable benchmarks" ON)

# Dipendenze
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost REQUIRED)


include(FetchContent)

# WebSocket++
FetchContent_Declare(
  websocketpp
  GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
)
FetchContent_MakeAvailable(websocketpp)

# ASIO (standalone)
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-28-0
)
FetchContent_MakeAvailable(asio)

# File sorgenti
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/**/*.cpp
)

# Executable
add_executable(arbitrage_bot ${SOURCES})

# Include directories
target_include_directories(arbitrage_bot PRIVATE
    ${CMAKE_SOURCE_DIR}/src/
    ${asio_SOURCE_DIR}/asio/include
    ${websocketpp_SOURCE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)


# Define macros
target_compile_definitions(arbitrage_bot PRIVATE ASIO_STANDALONE)

# Link libraries
target_link_libraries(arbitrage_bot
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
    ${Boost_LIBRARIES}
)

# Benchmark
if(BUILD_BENCHMARKS)
    # add_subdirectory(benchmark)
endif()

# Test
if(BUILD_TESTING)
    enable_testing()
    # add_subdirectory(test)
endif()
