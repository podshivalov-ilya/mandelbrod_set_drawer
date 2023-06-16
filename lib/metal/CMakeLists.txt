project(metalbrot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(METAL_KIT MetalKit)
find_library(METAL Metal)

# Find all Metal source files in current directory
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.metal)

# Set Metal compile flags
set(CMAKE_C_FLAGS "-x objective-c++ -fobjc-arc")

# Set variables for output files
set(AIR_OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.air)
set(METALLIB_OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.metallib)

# Compile all Metal files to .air and .metallib
add_custom_command(
    OUTPUT ${AIR_OUTPUT_FILE}
    COMMAND xcrun -sdk macosx metal -c ${SOURCES} -o ${AIR_OUTPUT_FILE}
    DEPENDS ${SOURCES}
)

add_custom_command(
    OUTPUT ${METALLIB_OUTPUT_FILE}
    COMMAND xcrun -sdk macosx metallib ${AIR_OUTPUT_FILE} -o ${METALLIB_OUTPUT_FILE}
    DEPENDS ${AIR_OUTPUT_FILE}
)

# Link target library with metallib file
add_custom_target(${PROJECT_NAME} ALL DEPENDS ${METALLIB_OUTPUT_FILE})
set(METALLIB ${METALLIB_OUTPUT_FILE} PARENT_SCOPE)
