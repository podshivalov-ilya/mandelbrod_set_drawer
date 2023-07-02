cmake_minimum_required(VERSION 3.23.3)

function (find_dearimgui)
    set(IMGUI_TARGET imgui)
    set(IMGUI_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui)
    set(IMGUI_INCLUDE_DIR ${IMGUI_SRC_DIR} PARENT_SCOPE)
    file(GLOB IMGUI_SOURCES
        ${IMGUI_SRC_DIR}/*.cpp
        ${IMGUI_SRC_DIR}/backends/imgui_impl_sdl2.cpp
        ${IMGUI_SRC_DIR}/backends/imgui_impl_sdlrenderer2.cpp
    )

    file(GLOB IMGUI_HEADERS
        ${IMGUI_SRC_DIR}/*.h
        ${IMGUI_SRC_DIR}/backends/imgui_impl_sdl2.h
        ${IMGUI_SRC_DIR}/backends/imgui_impl_sdlrenderer2.h
    )

    add_library(${IMGUI_TARGET} STATIC ${IMGUI_HEADERS} ${IMGUI_SOURCES})

    target_include_directories(${IMGUI_TARGET}
        PRIVATE
            ${IMGUI_SRC_DIR}
            ${SDL2_INCLUDE_DIR}
     )

     set_target_properties(${IMGUI_TARGET} PROPERTIES
         CXX_STANDARD 20
         CXX_STANDARD_REQUIRED ON
         COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
         LINKER_LANGUAGE CXX
         FOLDER 3rdparty
     )

     target_link_libraries(${IMGUI_TARGET}
        ${SDL2_LIBRARIES}
    )

    set(IMGUI_LIBRARIES ${IMGUI_TARGET} PARENT_SCOPE)
endfunction()

function (find_metal_cpp)
    if(NOT DEFINED METAL_CPP_INCLUDE_DIR)
      message(FATAL_ERROR "Path to metal-cpp include directory is not set!")
    endif()

    if(NOT EXISTS ${METAL_CPP_INCLUDE_DIR}/Metal/Metal.hpp)
      message(FATAL_ERROR "Path to metal-cpp include directory is wrong!")
    endif()

    set(METAL_CPP_LIB
        "-framework Metal"
        "-framework Foundation"
        "-framework QuartzCore"
        PARENT_SCOPE
    )
endfunction()

set(MAJOR_VERSION 0)
set(MINOR_VERSION 0)
set(PATCH_VERSION 999)
if($ENV{MAJOR_VERSION})
    set(MAJOR_VERSION $ENV{MAJOR_VERSION})
endif()
if($ENV{MINOR_VERSION})
    set(MINOR_VERSION $ENV{MINOR_VERSION})
endif()
if($ENV{CIRCLE_BUILD_NUM})
    set(PATCH_VERSION $ENV{CIRCLE_BUILD_NUM})
endif()

project(mandelbrot VERSION ${MAJOR_VERSION}.${MAJOR_VERSION}.${PATCH_VERSION})

if(NOT DEFINED HEADERS)
    file(GLOB HEADERS *.hpp)
endif()

if(NOT DEFINED SOURCES)
    file(GLOB SOURCES *.cpp)
endif()

find_package(SDL2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

set(METAL_CPP_INCLUDE_DIR "/usr/local/include/metal-cpp" CACHE PATH "Path to metal-cpp include directory")
find_metal_cpp()
find_dearimgui()

message("Dear ImGui include dir: ${IMGUI_INCLUDE_DIR}")
message("SDL2 include dir: ${SDL2_INCLUDE_DIR}")

source_group("Headers" FILES ${HEADERS})
source_group("Sources" FILES ${SOURCES})

add_subdirectory(lib/metal)

add_executable(${PROJECT_NAME})
add_dependencies(${PROJECT_NAME} metalbrot)

target_sources(${PROJECT_NAME}
    PRIVATE
        ${SOURCES}
    PUBLIC
        FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES "${HEADERS}"
)

target_link_libraries(${PROJECT_NAME}
    ${METAL_CPP_LIB}
    ${SDL2_LIBRARIES}
    ${IMGUI_LIBRARIES}
    fmt::fmt
    Eigen3::Eigen
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
)

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist.in)
    message(SEND_ERROR "File plist file is not specified")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist.in
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        METALLIB="${METALLIB}"
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${METAL_CPP_INCLUDE_DIR}
        ${IMGUI_INCLUDE_DIR}
        ${SDL2_INCLUDE_DIR}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
