
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -O5")

#FIND_PACKAGE(Boost 1.59.0 COMPONENTS program_options filesystem system REQUIRED)
#INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
SET(GTEST_SRC /home/jaronhalt/dev/googletest/googletest)
SET(GMOCK_SRC /home/jaronhalt/dev/googletest/googlemock)

ADD_LIBRARY(network SHARED network.cpp)

ADD_LIBRARY(metrics SHARED metrics.cpp timer.cpp counter.cpp )

TARGET_LINK_LIBRARIES(metrics network)

ADD_SUBDIRECTORY(test)


#TARGET_LINK_LIBRARIES(timer ${Boost_LIBRARIES})
