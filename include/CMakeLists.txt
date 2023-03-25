cmake_minimum_required(VERSION 3.19)

target_include_directories(kaos-public PUBLIC  
    $<INSTALL_INTERFACE:include>    
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

file(GLOB_RECURSE H_FILES "kaos-public/*.h")

target_sources(kaos-public PRIVATE ${H_FILES})

#install(DIRECTORY kaos-public DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
