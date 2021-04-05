cmake_minimum_required(VERSION 3.0)
project(nodefluxcv)
find_package(Boost REQUIRED system thread regex)  

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

add_library(boost_newest INTERFACE)
target_include_directories(boost_newest INTERFACE "${CMAKE_SOURCE_DIR}/include/boost_1_75_0/")
target_link_libraries(boost_newest INTERFACE pthread)

add_library(base64 INTERFACE)
target_include_directories(base64 INTERFACE "${CMAKE_SOURCE_DIR}/include/cpp-base64/")
target_link_libraries(base64 INTERFACE pthread)

add_library(asio INTERFACE)
target_compile_options(asio INTERFACE ASIO_STANDALONE)
target_include_directories(asio INTERFACE "${CMAKE_SOURCE_DIR}/include/asio-1.18.1/include")
target_link_libraries(asio INTERFACE pthread)

SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY} ${OpenCV_LIBS})

# image resizer test
set(SOURCE_TEST src/resizer_test.cpp)
add_executable(resizer_test ${SOURCE_TEST})
TARGET_LINK_LIBRARIES(resizer_test ${OpenCV_LIBS})

# client
set(SOURCE_CLIENT src/client.cpp)
add_executable(client ${SOURCE_CLIENT})
TARGET_LINK_LIBRARIES(client boost_newest ${OpenCV_LIBS})

# server
set(SOURCE_SERVER src/server.cpp)
add_executable(server ${SOURCE_SERVER})
TARGET_LINK_LIBRARIES(server boost_newest ${OpenCV_LIBS} base64)