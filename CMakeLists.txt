cmake_minimum_required(VERSION 3.10.0)

cmake_policy(SET CMP0015 NEW)

project(dxFeedCppSample)
set(CMAKE_CXX_STANDARD 11)

add_definitions(-DFMT_HEADER_ONLY=1)

set(dxFeedCppSample_wrappers src/wrappers/EventFlags.hpp src/wrappers/Candle.hpp src/wrappers/Quote.hpp
        src/wrappers/OrderScope.hpp src/wrappers/DXFeed.hpp src/wrappers/Connection.hpp src/wrappers/ConnectionStatus.hpp)
set(dxFeedCppSample_tools src/tools/EventsCollector.hpp src/tools/EventsDumper.hpp)
set(dxFeedCppSample_helpers src/tools/EventsCollector.hpp src/helpers/Handler.hpp)
set(dxFeedCppSample_converters src/converters/StringConverter.hpp
        src/converters/DateTimeConverter.hpp)

add_executable(dxFeedCppSample src/main.cpp ${dxFeedCppSample_converters} ${dxFeedCppSample_tools}
        ${dxFeedCppSample_helpers} ${dxFeedCppSample_wrappers})
target_include_directories(dxFeedCppSample PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/dxFeedCApi/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
target_link_directories(dxFeedCppSample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/dxFeedCApi/lib)
target_link_libraries(dxFeedCppSample PRIVATE debug DXFeedd_64 optimized DXFeed_64)

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
    set_target_properties(dxFeedCppSample PROPERTIES LINK_FLAGS "-Wl,-install_name,-rpath,@loader_path")
    add_compile_options(-Wall -Wextra -pedantic)
    add_custom_command(TARGET dxFeedCppSample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/thirdparty/dxFeedCApi/lib/libDXFeed_64.dylib
            ${CMAKE_CURRENT_BINARY_DIR}/libDXFeed_64.so)
    add_custom_command(TARGET dxFeedCppSample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/thirdparty/dxFeedCApi/lib/libDXFeedd_64.dylib
            ${CMAKE_CURRENT_BINARY_DIR}/libDXFeedd_64.so)
else ()
    set_target_properties(dxFeedCppSample PROPERTIES LINK_FLAGS "-Wl,-rpath,./")
    add_compile_options(-Wall -Wextra -pedantic)
    add_custom_command(TARGET dxFeedCppSample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/thirdparty/dxFeedCApi/lib/libDXFeed_64.so
            ${CMAKE_CURRENT_BINARY_DIR}/libDXFeed_64.so)
    add_custom_command(TARGET dxFeedCppSample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/thirdparty/dxFeedCApi/lib/libDXFeedd_64.so
            ${CMAKE_CURRENT_BINARY_DIR}/libDXFeedd_64.so)
    target_link_libraries(dxFeedCppSample PRIVATE pthread)
endif ()

include(FetchContent)
FetchContent_Declare(date_src
        GIT_REPOSITORY https://github.com/HowardHinnant/date.git
        GIT_TAG v3.0.1  # adjust tag/branch/commit as needed
        )
FetchContent_MakeAvailable(date_src)
target_link_libraries(dxFeedCppSample PRIVATE date::date)