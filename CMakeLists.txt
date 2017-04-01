cmake_minimum_required (VERSION 2.6)
set(PROJECT_NAME atmo)
project(${PROJECT_NAME})

# Set PulsarEngine path here
set(PULSAR_DIR ${CMAKE_SOURCE_DIR}/../PulsarEngine)

# Dir settings
add_subdirectory(${PULSAR_DIR} ${PULSAR_DIR}/build)
include_directories(${PULSAR_DIR}/include)
include_directories(include)

# Main executable target
file(GLOB SOURCES "src/*.cpp" "src/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} Pulsar)

# Copy data
add_custom_target(data_symlink ALL COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data)
if(EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --preload-file ${CMAKE_SOURCE_DIR}/data@/data")
endif()
