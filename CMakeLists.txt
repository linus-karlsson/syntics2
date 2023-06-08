cmake_minimum_required(VERSION 3.16.3) 
project(Syntics)
set(EXE ${PROJECT_NAME})

#-march=native -mtune=native
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
set(STANDARD_FLAGS "-nologo -EHa- -WX -W4 -wd4100 -wd4201 -wd4505")
#set(STANDARD_FLAGS "-W4 -WX -wd4100 -wd4201 -wd4505")
#set(STANDARD_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "${STANDARD_FLAGS} -Od -Oi -Z7 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${STANDARD_FLAGS} -O2 -Oi -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${STANDARD_FLAGS} -Od -Oi -Z7 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${STANDARD_FLAGS} -O2 -Oi -DNDEBUG")

option(INCLUDE_TEST "Include testning" OFF)

file(GLOB SOURCES "${PROJECT_NAME}/src/*.cc")
file(GLOB MATH "${PROJECT_NAME}/src/math/*.cc")
file(GLOB STB "${PROJECT_NAME}/vendor/stb/*.c")
file(GLOB OBJ "${PROJECT_NAME}/vendor/tiny-obj/*.cc")

IF (WIN32)
    file(GLOB PLATFORM "${PROJECT_NAME}/src/win32/*.cc")
ELSE()
    file(GLOB PLATFORM "${PROJECT_NAME}/src/linux/*.cc")
ENDIF()

if(INCLUDE_TEST)
    add_library(${EXE} STATIC ${SOURCES} ${MATH} ${STB} ${OBJ})
    add_subdirectory("${CMAKE_SOURCE_DIR}/Test")
    include(CTest)
    add_test("Test" "Test/bin/Test" COMMAND "Test/bin/Test")
else()
    add_executable(${EXE} ${SOURCES} ${MATH} ${STB} ${OBJ} ${PLATFORM})
endif()


IF (WIN32)

    target_include_directories(${EXE}
        PUBLIC "${CMAKE_SOURCE_DIR}"
        PUBLIC "${CMAKE_SOURCE_DIR}\\${PROJECT_NAME}\\src"
        PUBLIC "${CMAKE_SOURCE_DIR}\\${PROJECT_NAME}\\vendor"
        PUBLIC "${CMAKE_SOURCE_DIR}\\${PROJECT_NAME}\\vendor\\glm"
        PUBLIC "${CMAKE_SOURCE_DIR}\\${PROJECT_NAME}\\vendor\\freetype2/include"
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

    target_link_libraries(${EXE} vulkan-1 user32 Winmm pdh)


    option(WINDOW_32 "Windows" ON)
    option(LINUX "Linux" OFF)

ELSE()

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

    option(WINDOW_32 "Windows" OFF)
    option(LINUX "Linux" ON)
ENDIF()

#target_precompile_headers(${EXE} PUBLIC "${PROJECT_NAME}/src/stdafx.h")
