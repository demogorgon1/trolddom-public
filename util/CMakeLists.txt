cmake_minimum_required (VERSION 3.19)

file(GLOB CPP_FILES "*.cpp")
file(GLOB H_FILES "*.h")

add_executable(tpublic-util ${CPP_FILES} ${H_FILES})

if(TROLDDOM_PUBLIC_PRECOMPILED_HEADERS)
	target_precompile_headers(tpublic-util PRIVATE "Pcheader.h")
endif()

target_compile_features(tpublic-util PRIVATE cxx_std_20)
target_link_libraries(tpublic-util tpublic::tpublic tpublic-extra::tpublic-extra)

