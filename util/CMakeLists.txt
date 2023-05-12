cmake_minimum_required (VERSION 3.19)

file(GLOB CPP_FILES "*.cpp")
file(GLOB H_FILES "*.h")

add_executable(kpublic-util ${CPP_FILES} ${H_FILES})

if(KAOS_PUBLIC_PRECOMPILED_HEADERS)
	target_precompile_headers(kpublic-util PRIVATE "Pcheader.h")
endif()

target_compile_features(kpublic-util PRIVATE cxx_std_20)
target_link_libraries(kpublic-util tpublic::tpublic tpublic-extra::tpublic-extra)

