cmake_minimum_required(VERSION 3.10)

project(app)

set(LIBUV_BUILD_TESTS ON CACHE BOOL "Build libuv tests")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. libuv_build)

set(SRC_LIST
    base/sem.cpp
    poller/asynccontext.cpp
    poller/asynccontext.h
    poller/loopcontext.cpp
    poller/loopcontext.h
    poller/eventpoller.cpp
    poller/eventpoller.h
)

find_package(spdlog REQUIRED)

add_executable(app app/main.cpp ${SRC_LIST})
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(app PRIVATE libuv::libuv spdlog::spdlog)