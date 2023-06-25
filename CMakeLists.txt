cmake_minimum_required(VERSION 3.22)

set(CPM_DOWNLOAD_VERSION 0.35.5)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake")
    file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif()

include(${CPM_DOWNLOAD_LOCATION})

project(coremud)

set(CMAKE_CXX_STANDARD 23)

CPMAddPackage("gh:gabime/spdlog#v1.11.0")
CPMAddPackage("gh:nlohmann/json#v3.11.2")
CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:robinlinden/libsodium-cmake#master")
CPMAddPackage("gh:SRombauts/SQLiteCpp#3.3.0")
CPMAddPackage("gh:skypjack/entt#v3.11.1")

find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system program_options regex)

file(GLOB_RECURSE COREMUD_INCLUDE include/*.h)
file(GLOB_RECURSE COREMUD_SRC src/*.cpp)

SET(COREMUD_INCLUDE_DIRS
        include
        ${Boost_INCLUDE_DIRS}
        ${spdlog_SOURCE_DIR}/include
        ${nlohmann_json_SOURCE_DIR}/single_include
        ${SQLiteCpp_SOURCE_DIR}/include
        ${sodium_SOURCE_DIR}/libsodium/src/libsodium/include
        ${FMT_SOURCE_DIR}/include
        ${entt_SOURCE_DIR}/single_include
)

add_library(coremud ${COREMUD_INCLUDE} ${COREMUD_SRC})
target_include_directories(coremud PUBLIC ${COREMUD_INCLUDE_DIRS})
set(COREMUD_LINK_LIBRARIES ${SQLite3_LIBRARIES} SQLiteCpp ${Boost_LIBRARIES} sodium fmt::fmt)
target_link_libraries(coremud ${COREMUD_LINK_LIBRARIES})