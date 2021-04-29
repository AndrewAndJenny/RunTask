cmake_minimum_required (VERSION 3.0)
project(RunTask CXX)

#set
set(BOOST_INCLUDE_DIRS $ENV{BOOST_LIBRARY_DIR} cache path "BOOST_INCLUDE_DIRS")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(PLATFORM "Linux")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
	if(CMAKE_CL_64)  
		set(PLATFORM "x64")
	else(CMAKE_CL_64)  
		set(PLATFORM "WIN32")
	endif(CMAKE_CL_64)
else()
	set(PLATFORM "others")
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${PLATFORM})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${PLATFORM})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${PLATFORM})

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
#boost
find_package(Boost REQUIRED)

if(BOOST_NOT_FOUND)
	message(STATUS "couldn't find Boost")
endif(BOOST_NOT_FOUND)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

#qt5
find_package(Qt5 COMPONENTS Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

file(GLOB SOURCES "*.cpp" "*.h" "*.hpp")
set(SRCS_FILES ${SOURCES})

add_executable (${PROJECT_NAME} ${SRCS_FILES})
target_link_libraries( ${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} -lboost_filesystem -lboost_system)