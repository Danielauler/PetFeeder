cmake_minimum_required(VERSION 2.8.4)
project(ComedouroBot)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lwiringPi -Wall")
set(Boost_USE_MULTITHREADED ON)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(CURL)
include_directories(/usr/local/include ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif()

add_executable(ComedouroBot main.cpp scanBowl.cpp)

target_link_libraries(ComedouroBot /usr/local/lib/libTgBot.a ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${CURL_LIBRARIES} ${OpenCV_LIBS})
