cmake_minimum_required(VERSION 3.10)


project("curl_download")

include_directories(./include/)

file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)


add_compile_options(-g)
add_executable(curl_download ${SRC})

add_executable(demo ${CMAKE_CURRENT_SOURCE_DIR}/src/demo.c)
target_link_libraries(demo pthread curl)

target_link_libraries(curl_download pthread curl)


