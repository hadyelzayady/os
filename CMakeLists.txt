cmake_minimum_required(VERSION 3.8)
project(Code)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        #clk.cpp
        #FilesGenerator.cpp
        headers.h
        #main.cpp
        # process.cpp
        processGenerator.cpp
        #scheduler.cpp
        process.h.h)

add_executable(Code ${SOURCE_FILES})