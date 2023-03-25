cmake_minimum_required (VERSION 3.19)

file(GLOB_RECURSE CPP_FILES "*.cpp")
file(GLOB_RECURSE H_FILES "*.h")

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(
		-Wall 
		-Wextra 
		-pedantic 
		-Werror 
		-Wno-unused-function 	# I want unused functions for debugging purposes
		-Wno-reorder			# I want to be able to initialize class members in any order
		-Wno-class-memaccess	# I want to be able to memset() things that I know are safe to memset()
	)	
endif()
	
add_library(kaos-public STATIC ${CPP_FILES} ${H_FILES})
add_library(kaos-public::kaos-public ALIAS kaos-public)


target_compile_features(kaos-public PRIVATE cxx_std_20)

if(KAOS_PUBLIC_PRECOMPILED_HEADERS)
	target_precompile_headers(kaos-public PRIVATE "Pcheader.h")
endif()
	