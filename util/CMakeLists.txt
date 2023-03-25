cmake_minimum_required (VERSION 3.19)

file(GLOB CPP_FILES "*.cpp")
file(GLOB H_FILES "*.h")

add_executable(kaos-public-util ${CPP_FILES} ${H_FILES})

if(KAOS_PUBLIC_PRECOMPILED_HEADERS)
	target_precompile_headers(kaos-public-util PRIVATE "Pcheader.h")
endif()

target_compile_features(kaos-public-util PRIVATE cxx_std_20)
target_link_libraries(kaos-public-util kaos-public::kaos-public)

