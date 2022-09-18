cmake_minimum_required(VERSION 3.16.3) 
project(Test)
set(EXE ${PROJECT_NAME})

#-march=native -mtune=native
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(STANDARD_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "${STANDARD_FLAGS} -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${STANDARD_FLAGS} -O2 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${STANDARD_FLAGS} -g -fPIC -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${STANDARD_FLAGS} -O2 -pedantic -DNDEBUG")

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/src/*.cpp")
add_executable(${EXE} ${SOURCES})

target_include_directories(${EXE}
    PUBLIC "${CMAKE_SOURCE_DIR}"
    PUBLIC "${CMAKE_SOURCE_DIR}/Syntics/src"
)

target_link_libraries(${EXE} Syntics)

