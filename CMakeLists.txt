cmake_minimum_required(VERSION 2.8)
project(coroutines)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++2a -stdlib=libc++")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN/'")

set(SOURCE_FILES main.cpp)

SET(LIBRARY_OUTPUT_PATH "${coroutines_SOURCE_DIR}/${CMAKE_BUILD_TYPE}")

SET(EXECUTABLE_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH}")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

#target_link_libraries(${PROJECT_NAME} rt dl pthread)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}"
)
