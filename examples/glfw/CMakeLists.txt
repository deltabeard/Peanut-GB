# vim: ts=4:sw=4:expandtab
CMAKE_MINIMUM_REQUIRED(VERSION 3.14...3.26 FATAL_ERROR)

# Initialise project information.
PROJECT(peanut-glfw)

#find_package(glfw3 3.3 REQUIRED)

ADD_EXECUTABLE(${PROJECT_NAME} main.c gb.c gl.c)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE inc)
TARGET_LINK_LIBRARIES(peanut-glfw glfw3)
