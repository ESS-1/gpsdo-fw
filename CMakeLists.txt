cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME gpsdo)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

add_subdirectory(lib)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    src/main.c
    src/eeprom.c
    src/frequency.c
    src/gps.c
    src/int.c
    src/bootlog.c
    src/cdcio.c
    src/pll.c
    src/pll_presets.c
    src/icons.c
    src/ui_core.c
    src/ui_helpers.c
    src/ui_msgbox.c
    src/ui.c
    src/encoder.c
    src/timer.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    eeprom
    st7735
    si5351
)

# Include firmware version and type definition file
include("${CMAKE_CURRENT_SOURCE_DIR}/version.cmake")

# Construct output filename
set(OUTPUT_FILE_NAME "${FW_MODEL}--${FW_VERSION}-${FW_TYPE}-${CMAKE_BUILD_TYPE}")

# Set the output filename for the executable target (.elf)
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES 
    OUTPUT_NAME "${OUTPUT_FILE_NAME}"
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Pass the model, FW version, and FW type to compiler
    BUILD_FW_VERSION="${FW_VERSION}"
    BUILD_FW_TYPE="${FW_TYPE}"
    BUILD_FW_MODEL="${FW_MODEL}"
)

# Generate '.bin' from the compiled '.elf'
add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE_NAME}.bin
    COMMENT "Generating BIN file: ${OUTPUT_FILE_NAME}.bin"
)
