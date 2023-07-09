cmake_minimum_required(VERSION 3.16.3) 

#set(CMAKE_C_COMPILER "clang")
#set(CMAKE_CXX_COMPILER "clang++")

project(Syntics)
set(EXE ${PROJECT_NAME})

#-march=native -mtune=native
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
set(STANDARD_FLAGS "-nologo -EHa- -W4 -wd4100 -wd4201 -wd4505")
#set(STANDARD_FLAGS "-W4 -WX -wd4100 -wd4201 -wd4505")
#set(STANDARD_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "${STANDARD_FLAGS} -Od -Oi -Zi -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${STANDARD_FLAGS} -O2 -Oi -Zi -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${STANDARD_FLAGS} -Od -Oi -Zi -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${STANDARD_FLAGS} -O2 -Oi -Zi -DNDEBUG")

#set(STANDARD_FLAGS "-Wno-null-dereference")
#set(CMAKE_CXX_FLAGS_DEBUG "${STANDARD_FLAGS} -g -DDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE "${STANDARD_FLAGS} -O2 -DNDEBUG")
#set(CMAKE_C_FLAGS_DEBUG "${STANDARD_FLAGS}  -g -fPIC -DDEBUG")
#set(CMAKE_C_FLAGS_RELEASE "${STANDARD_FLAGS} -O2 -pedantic -DNDEBUG")

option(INCLUDE_TEST "Include testning" OFF)

set(SOURCES "${PROJECT_NAME}/src/syntics.c")
set(STB "${PROJECT_NAME}/vendor/stb/stb_image.c")
#set(OBJ "${PROJECT_NAME}/vendor/tiny-obj/tiny_obj_loader.cc")
#file(GLOB SOURCES "${PROJECT_NAME}/src/syntics.cc")
#file(GLOB MATH "${PROJECT_NAME}/src/math/*.cc")
#file(GLOB STB "${PROJECT_NAME}/vendor/stb/stb_image.c")
#file(GLOB OBJ "${PROJECT_NAME}/vendor/tiny-obj/*.cc")

IF (WIN32)
    #file(GLOB PLATFORM "${PROJECT_NAME}/src/win32/*.cc")
ELSE()
    #file(GLOB PLATFORM "${PROJECT_NAME}/src/linux/*.cc")
ENDIF()

if(INCLUDE_TEST)
    add_library(${EXE} STATIC ${SOURCES} ${STB} ${OBJ})
    add_subdirectory("${CMAKE_SOURCE_DIR}/Test")
    include(CTest)
    add_test("Test" "Test/bin/Test" COMMAND "Test/bin/Test")
else()
    add_executable(${EXE} ${SOURCES} ${STB} ${OBJ} ${PLATFORM})
endif()


IF (WIN32)

    target_include_directories(${EXE}
        PUBLIC "${CMAKE_SOURCE_DIR}"
        PUBLIC "${CMAKE_SOURCE_DIR}\\${PROJECT_NAME}\\headers"
        PUBLIC "${CMAKE_SOURCE_DIR}\\${PROJECT_NAME}\\src"
        PUBLIC "${CMAKE_SOURCE_DIR}\\${PROJECT_NAME}\\vendor"
        PUBLIC "C:\\VulkanSDK\\1.3.236.0\\Include"
		PUBLIC "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.34.31933\\include"
    )

#External dep
#add_subdirectory("${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/vendor/freetype2")

# Link External dep
    target_link_directories(${EXE}
        PRIVATE "C:\\VulkanSDK\\1.3.236.0\\Lib"
    )

# For entrypoint at WinMain 
target_link_options(${EXE} PRIVATE "/SUBSYSTEM:WINDOWS") 

    # For gcc and g++, clang
    #target_link_options(${EXE} PRIVATE "-mwindows") 

    target_link_libraries(${EXE} vulkan-1 user32 Winmm pdh)


    option(WINDOW_32 "Windows" ON)
    option(LINUX "Linux" OFF)

ELSE()

    target_include_directories(${EXE}
        PUBLIC "${CMAKE_SOURCE_DIR}"
        PUBLIC "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/src"
        PUBLIC "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/vendor"
    )

#External dep
#add_subdirectory("${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/vendor/freetype2")

# Link External dep
#target_link_directories(${EXE}
#    PRIVATE "build/${PROJECT_NAME}/vendor/freetype2"
#)

    target_link_libraries(${EXE} vulkan xcb xcb-xfixes X11 X11-xcb xcb-cursor)

    option(WINDOW_32 "Windows" OFF)
    option(LINUX "Linux" ON)
ENDIF()

#target_precompile_headers(${EXE} PUBLIC "${PROJECT_NAME}/src/stdafx.h")
