cmake_minimum_required(VERSION 3.10)
project(cpp-websocket-chatroom LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system thread)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found")
endif()

# If websocketpp headers are not installed system-wide, set WEBSOCKETPP_INCLUDE_DIR to that path before running cmake
if(NOT DEFINED WEBSOCKETPP_INCLUDE_DIR)
    set(WEBSOCKETPP_INCLUDE_DIR "/usr/include") # adjust if needed
endif()

include_directories(${Boost_INCLUDE_DIRS} ${WEBSOCKETPP_INCLUDE_DIR})

add_executable(chat_server server/main.cpp)

target_include_directories(chat_server PRIVATE ${WEBSOCKETPP_INCLUDE_DIR})
target_link_libraries(chat_server PRIVATE ${Boost_LIBRARIES})
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(chat_server PRIVATE Threads::Threads)
