CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT(RollDiffTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

set(SourceFiles 
	main.cpp
	test_data.h
	tst_hash.h
	tst_hash_roll.h
)

message("SourceFiles:  ${SourceFiles}")

# create a group inside the Visual Studio IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourceFiles})

add_executable(${PROJECT_NAME} ${SourceFiles})


target_link_libraries(${PROJECT_NAME} 
	GTest::gtest_main
	RollDiff
)


include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/data"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data")
