project(RollDiff)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SourceFiles 
	checksum.hpp
	hash.hpp
	signature.hpp
	signature.cpp
	delta.hpp
	delta.cpp
	patch.hpp
)

add_library(${PROJECT_NAME} ${SourceFiles})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Src})
