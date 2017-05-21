CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(TaskQueue)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
   message(WARNING "In-source building is discouraged!")
endif("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")

ADD_EXECUTABLE(TaskQueue
	src/taskqueue.hpp
	src/test.cpp
)
