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
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

CPMAddPackage("gh:gabime/spdlog#v1.11.0")
CPMAddPackage("gh:nlohmann/json#v3.11.2")

find_package(Boost REQUIRED COMPONENTS system)

file(GLOB_RECURSE COREMUD_INCLUDE include/*.h)
file(GLOB_RECURSE COREMUD_SRC src/*.cpp)

SET(COREMUD_INCLUDE_DIRS
        include
        ${Boost_INCLUDE_DIRS}
        ${spdlog_SOURCE_DIR}/include
        ${nlohmann_json_SOURCE_DIR}/single_include
)

add_library(coremud ${COREMUD_INCLUDE} ${COREMUD_SRC})
target_include_directories(coremud PUBLIC ${COREMUD_INCLUDE_DIRS})
set(COREMUD_LINK_LIBRARIES ${Boost_LIBRARIES})
target_link_libraries(coremud ${COREMUD_LINK_LIBRARIES})