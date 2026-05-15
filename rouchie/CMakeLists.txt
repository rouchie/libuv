cmake_minimum_required(VERSION 3.10)

project(app)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. libuv_build)

set(SRC_LIST
    base/sem.cpp
    base/event.cpp
    base/uv_scheduler.cpp
    base/task.cpp
    base/task_pool.cpp
)

find_package(spdlog REQUIRED)

add_executable(app app/main.cpp ${SRC_LIST})
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(app PRIVATE libuv::libuv spdlog::spdlog)