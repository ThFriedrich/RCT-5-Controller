cmake_minimum_required(VERSION 3.16.0)

project(RCT_5_Control LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

MESSAGE(STATUS "Using ${CMAKE_CXX_COMPILER_ID} Compiler!")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if (UNIX)
    set(SDL2_INCLUDE_DIR /usr/include/SDL2)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    # SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    # SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
endif(UNIX)

if (WIN32)
    set(SDL2_INCLUDE_DIR C:/SDL2_VC/include/)
    set(SDL2_LIB_DIR C:/SDL2_VC/lib/x64)
    set(BUILD_DIR ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    MESSAGE(STATUS ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    link_directories(${SDL2_LIB_DIR})

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread -std=c++17")
        set(CMAKE_CXX_FLAGS_DEBUG "-g")
        set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    endif()

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /nologo")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GL")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GA")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Qpar")
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /favor:INTEL64")
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
    endif()

    file(COPY ${SDL2_LIB_DIR}/SDL2.dll DESTINATION ${BUILD_DIR})
endif (WIN32)

add_executable(RCT_5_Control src/main.cpp)

target_sources( RCT_5_Control PRIVATE
    imgui/misc/cpp/imgui_stdlib.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl2.cpp
    imgui/backends/imgui_impl_sdlrenderer2.cpp
    # imgui/imgui_demo.cpp
    imgui/imgui.cpp
    implot/implot.cpp
    implot/implot_items.cpp
    src/SerialPort.cpp
    src/RCT_5_Control.cpp
    src/NamurCommands.cpp
    src/ImGuiINI.hpp
    src/Utilities.cpp
    src/Timeline.cpp
    src/beeper.cpp
    src/FileOperations.cpp
    )

target_include_directories( RCT_5_Control PUBLIC
    imgui
    imgui/backends
    imgui/misc/cpp
    implot
    ${SDL2_INCLUDE_DIR}
    include
    src
    )

target_link_libraries(RCT_5_Control PUBLIC SDL2main SDL2 ${CMAKE_DL_LIBS})