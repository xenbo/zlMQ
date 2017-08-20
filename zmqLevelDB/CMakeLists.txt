cmake_minimum_required(VERSION 3.8)
project(zmqLevelDB)

set(CMAKE_CXX_STANDARD 98)

include_directories(. leveldb)
link_directories(.)

set(SOURCE_FILES Testmain.cpp getcmdFromzmq.cpp getcmdFromzmq.h job.cpp job.h opleveldb.cpp opleveldb.h zmq.h zmqpipe.cpp zmqpipe.h pushout.h pushout.cpp)
add_executable(zmqLevelDB ${SOURCE_FILES})

target_link_libraries(zmqLevelDB zmq leveldb pthread rt)