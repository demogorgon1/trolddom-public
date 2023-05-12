cmake_minimum_required(VERSION 3.19)

target_include_directories(tpublic PUBLIC  
    $<INSTALL_INTERFACE:include>    
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

file(GLOB_RECURSE H_FILES "tpublic/*.h")

target_sources(tpublic PRIVATE ${H_FILES})

install(DIRECTORY tpublic DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
