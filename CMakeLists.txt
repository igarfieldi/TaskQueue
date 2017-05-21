CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(TaskQueue)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
   message(WARNING "In-source building is discouraged!")
endif("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")

set(CMAKE_CXX_STANDARD 14)

ADD_EXECUTABLE(TaskQueue
	src/taskqueue.hpp
	src/test.cpp
)
