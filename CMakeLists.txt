cmake_minimum_required(VERSION 3.10)

project(rimo_zoom_bot_manager VERSION 1.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(Boost REQUIRED)

target_compile_options(${PROJECT_NAME} PRIVATE
    -O0
    -Wall
    -Wshadow
    -Wunused-result
    -Wextra
    -Wpedantic
    -Wconversion
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast
)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})

include_directories(${CMAKE_SOURCE_DIR}/include)

# add_custom_command(
#     OUTPUT ${CMAKE_SOURCE_DIR}/compile_commands.json
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         ${CMAKE_BINARY_DIR}/compile_commands.json
#         ${CMAKE_SOURCE_DIR}/compile_commands.json
#     DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
#     COMMENT "Copying compile_commands.json to the project root directory"
# )
