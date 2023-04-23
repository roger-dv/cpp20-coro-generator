cmake_minimum_required(VERSION 3.19)

#set(CMAKE_C_COMPILER "gcc")
#set(CMAKE_CXX_COMPILER "g++")

# set option to link the C++ std lib (for g++ or clang++)
if(DEFINED CMAKE_CXX_COMPILER)
    message(STATUS "CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
    if(CMAKE_CXX_COMPILER MATCHES "^(.*/)?g\\+\\+$")
        set(CXX_LIB_OPTN "-static-libstdc++")
    elseif(CMAKE_CXX_COMPILER MATCHES "^(.*/)?clang\\+\\+$")
        set(CXX_LIB_OPTN "-stdlib=libc++")
    else()
        message(STATUS "unknown C++ compiler: CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
    endif()
endif()
message(STATUS "CXX_LIB_OPTN=${CXX_LIB_OPTN}")

project(coroutines)

if(DEFINED ENV{OS} AND "$ENV{OS}" STREQUAL "Windows_NT")
    set(OS_PLATFORM "win32")
else()
    set(OS_PLATFORM "linux")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wno-unknown-pragmas -std=c11")

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unknown-pragmas -std=c++20 ${CXX_LIB_OPTN}")
message(STATUS "CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN/'")

set(SOURCE_FILES main.cpp)

SET(LIBRARY_OUTPUT_PATH "${coroutines_SOURCE_DIR}/${CMAKE_BUILD_TYPE}")

SET(EXECUTABLE_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH}")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

#target_link_libraries(${PROJECT_NAME} rt dl pthread)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}"
)
