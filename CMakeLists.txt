cmake_minimum_required(VERSION 2.6)

PROJECT(ubox-examples C)
ADD_DEFINITIONS(-O1 --std=gnu99 -g3 -lm)

IF(APPLE)
  INCLUDE_DIRECTORIES(/opt/local/include)
  LINK_DIRECTORIES(/opt/local/lib)
ENDIF()

INCLUDE_DIRECTORIES(/opt/git/libubox)
LINK_DIRECTORIES(/opt/git/libubox)


ADD_EXECUTABLE(ustream-example ustream-example.c)
TARGET_LINK_LIBRARIES(ustream-example ubox)

