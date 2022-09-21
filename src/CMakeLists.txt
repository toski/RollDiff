CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT(RollDiff LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_SHARED_LIBS "Should HashDiff be a shered library?" OFF)
option(BUILD_TESTS "Should we build the test project?" ON)


include_directories("${CMAKE_SOURCE_DIR}/lib/")

add_subdirectory(lib)
add_subdirectory(app)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()