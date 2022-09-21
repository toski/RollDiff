CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT(RollDiffApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(SourceFiles 
	main.cpp
)

# create a group inside the Visual Studio IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourceFiles})

add_executable(${PROJECT_NAME} ${SourceFiles})


target_link_libraries(${PROJECT_NAME} 
	RollDiff
)

