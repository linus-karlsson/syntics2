cmake_minimum_required(VERSION 3.16.3) 
project(syntics2)
set(EXE exe_name)

#-march=native -mtune=native
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(STANDARD_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "${STANDARD_FLAGS} -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${STANDARD_FLAGS} -O2 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${STANDARD_FLAGS} -g -fPIC -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${STANDARD_FLAGS} -O2 -pedantic -DNDEBUG")

file(GLOB SOURCES "${PROJECT_NAME}/src/*.cpp")
add_executable(${EXE} ${SOURCES} ${STB} ${TESTING})

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
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


target_include_directories(${EXE}
    PUBLIC "${PROJECT_SOURCE_DIR}"
)

#External dep
#add_subdirectory("${PROJECT_NAME}/vendor/")

# Link External dep
#target_link_directories(${EXE}
#    PRIVATE "${PROJECT_NAME}/vendor/"
#)

target_link_libraries(${EXE} vulkan xcb)

#target_precompile_headers(${EXE} PUBLIC "${PROJECT_NAME}/src/stdafx.h")
