cmake_minimum_required (VERSION 3.0)
project(RunTask CXX)

#set
if(MSVC)
set(BOOST_INCLUDEDIR $ENV{Boost_INCLUDE_DIRS} CACHE PATH "BOOST_INCLUDEDIR")
set(BOOST_LIBRARYDIR $ENV{BOOST_LIBRARY_DIR} CACHE PATH "BOOST_LIBRARYDIR")
endif()
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# set MD to MT
if(OPTION_BUILD_MT)
set(CompilerFlags
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
  )

foreach(CompilerFlags ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlags} "${${CompilerFlags}}")
endforeach()
endif()

#qt5
find_package(Qt5 COMPONENTS Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADS ${CMAKE_SOURCE_DIR}/include/*.h ${CMAKE_SOURCE_DIR}/include/*.hpp)
set(SRCS ${SOURCES} ${HEADS})
add_executable (${PROJECT_NAME} ${SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core )