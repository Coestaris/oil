cmake_minimum_required(VERSION 3.13)
project(oil C)

set(CMAKE_C_STANDARD 11)

add_executable(oil main.c oil.h png.c png.h oilerror.c oilerror.h crc32.c crc32.h graphics.c graphics.h main.c bmp.c bmp.h common.c common.h bmpSizes.h)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

target_link_libraries(oil m)
target_link_libraries(oil z)
target_link_libraries(oil ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

