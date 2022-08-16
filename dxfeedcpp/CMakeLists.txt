cmake_minimum_required(VERSION 3.10)

project(dxfeedcpp VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 11)

add_library(dxfeedcpp INTERFACE)
add_library(dxfeedcpp::dxfeedcpp ALIAS dxfeedcpp)

add_subdirectory(include/common)
add_subdirectory(include/connections)
add_subdirectory(include/converters)
add_subdirectory(include/events)
add_subdirectory(include/helpers)
add_subdirectory(include/processors)
add_subdirectory(include/subscriptions)
add_subdirectory(include/utils)

target_link_libraries(dxfeedcpp INTERFACE dxfeedcpp::common dxfeedcpp::connections dxfeedcpp::converters
        dxfeedcpp::events dxfeedcpp::helpers dxfeedcpp::processors dxfeedcpp::subscriptions dxfeedcpp::utils)

target_include_directories(dxfeedcpp INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")