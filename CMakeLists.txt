cmake_minimum_required(VERSION 3.10.0)

cmake_policy(SET CMP0015 NEW)

project(dxFeedCppSample)
set(CMAKE_CXX_STANDARD 11)

#add_definitions(-DFMT_HEADER_ONLY=1)
add_executable(dxFeedCppSample src/main.cpp)

add_subdirectory(thirdparty/date-3.0.1)
add_subdirectory(dxfeedcpp)

target_include_directories(dxFeedCppSample PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/dxFeedCApi/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_link_directories(dxFeedCppSample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/dxFeedCApi/lib)
target_link_libraries(dxFeedCppSample PRIVATE debug DXFeedd_64 optimized DXFeed_64)
target_link_libraries(dxFeedCppSample PRIVATE date::date dxfeedcpp::dxfeedcpp)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
    add_definitions(-DMACOSX)
endif ()

if (WIN32)
    if (MSVC)
        add_compile_options(/W4 /WX)
    elseif (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
        add_compile_options(-Wall -Wextra -pedantic)
    endif ()
    add_custom_command(TARGET dxFeedCppSample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/thirdparty/dxFeedCApi/lib/DXFeed_64.dll
            ${CMAKE_CURRENT_BINARY_DIR}/DXFeed_64.dll)
    add_custom_command(TARGET dxFeedCppSample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/thirdparty/dxFeedCApi/lib/DXFeedd_64.dll
            ${CMAKE_CURRENT_BINARY_DIR}/DXFeedd_64.dll)
elseif (MACOSX)
    set_target_properties(dxFeedCppSample PROPERTIES MACOSX_RPATH TRUE)
    #set_target_properties(dxFeedCppSample PROPERTIES LINK_FLAGS "-Wl,-install_name,-rpath,@loader_path")
    add_compile_options(-Wall -Wextra -pedantic)
    add_custom_command(TARGET dxFeedCppSample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/thirdparty/dxFeedCApi/lib/libDXFeed_64.dylib
            ${CMAKE_CURRENT_BINARY_DIR}/libDXFeed_64.dylib)
    add_custom_command(TARGET dxFeedCppSample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/thirdparty/dxFeedCApi/lib/libDXFeedd_64.dylib
            ${CMAKE_CURRENT_BINARY_DIR}/libDXFeedd_64.dylib)
else ()
    set_target_properties(dxFeedCppSample PROPERTIES LINK_FLAGS "-Wl,-rpath,./")
    add_compile_options(-Wall -Wextra -pedantic)
    add_custom_command(TARGET dxFeedCppSample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/thirdparty/dxFeedCApi/lib/libDXFeed_64.so
            ${CMAKE_CURRENT_BINARY_DIR}/libDXFeed_64.so)
    add_custom_command(TARGET dxFeedCppSample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/thirdparty/dxFeedCApi/lib/libDXFeedd_64.so
            ${CMAKE_CURRENT_BINARY_DIR}/libDXFeedd_64.so)
endif ()

if (NOT MSVC)
    find_package(Threads)
    target_link_libraries(dxFeedCppSample PUBLIC Threads::Threads)
endif ()