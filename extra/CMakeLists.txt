cmake_minimum_required (VERSION 3.19)

file(GLOB_RECURSE CPP_FILES "*.cpp")
file(GLOB_RECURSE H_FILES "*.h")
	
add_library(kaos-public-extra STATIC ${CPP_FILES} ${H_FILES})
add_library(kaos-public-extra::kaos-public-extra ALIAS kaos-public-extra)
	