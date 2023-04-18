cmake_minimum_required(VERSION 3.19)

target_include_directories(kpublic PUBLIC  
    $<INSTALL_INTERFACE:include>    
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

file(GLOB_RECURSE H_FILES "kpublic/*.h")

target_sources(kpublic PRIVATE ${H_FILES})

install(DIRECTORY kpublic DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
