cmake_minimum_required(VERSION 3.16.3) 
project(Syntics)
set(EXE ${PROJECT_NAME})

#-march=native -mtune=native
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
#set(STANDARD_FLAGS "-Wall -Wno-unused-function -Wno-unused-variable")
set(STANDARD_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "${STANDARD_FLAGS} -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${STANDARD_FLAGS} -O3 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${STANDARD_FLAGS} -g -fPIC -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${STANDARD_FLAGS} -O3 -pedantic -DNDEBUG")

option(INCLUDE_TEST "Include testning" OFF)

file(GLOB SOURCES "${PROJECT_NAME}/src/*.cpp")
file(GLOB MATH "${PROJECT_NAME}/src/math/*.cpp")
file(GLOB STB "${PROJECT_NAME}/vendor/stb/*.cpp")
file(GLOB OBJ "${PROJECT_NAME}/vendor/tiny-obj/*.cpp")

if(INCLUDE_TEST)
    add_library(${EXE} STATIC ${SOURCES} ${MATH} ${STB} ${OBJ})
    add_subdirectory("${CMAKE_SOURCE_DIR}/Test")
    include(CTest)
    add_test("Test" "Test/bin/Test" COMMAND "Test/bin/Test")
else()
    add_executable(${EXE} ${SOURCES} ${MATH} ${STB} ${OBJ})
endif()


target_include_directories(${EXE}
    PUBLIC "${CMAKE_SOURCE_DIR}"
    PUBLIC "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/src"
    PUBLIC "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/vendor"
    PUBLIC "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/vendor/glm"
    PUBLIC "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/vendor/freetype2/include"
)

#External dep
#add_subdirectory("${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/vendor/freetype2")

# Link External dep
#target_link_directories(${EXE}
#    PRIVATE "build/${PROJECT_NAME}/vendor/freetype2"
#)

target_link_libraries(${EXE} vulkan xcb xcb-xfixes X11 X11-xcb xcb-cursor)

if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Check Submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} RESULT_VARIABLE GIT_SUBMOD_RESULT
        )
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed")
        endif()
    endif()
endif()

#if(NOT EXISTS "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/vendor/glfw/CMakeLists.txt")
#    message(FATAL_ERROR "glfw submodule does not exist, GIT_SUBMODULE is off. Either turn it on or run command: submodule update --init --recursive in terminal on project directory")
#endif()

#target_precompile_headers(${EXE} PUBLIC "${PROJECT_NAME}/src/stdafx.h")
