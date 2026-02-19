cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME gpsdo)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Define EEPROM_AUTO_SAVE
if(NOT DEFINED EEPROM_AUTO_SAVE)
  message(FATAL_ERROR "EEPROM_AUTO_SAVE must be defined (0 or 1)")
endif()

if(EEPROM_AUTO_SAVE STREQUAL "1")
  add_compile_definitions(EEPROM_AUTO_SAVE=1)
elseif(EEPROM_AUTO_SAVE STREQUAL "0")
  add_compile_definitions(EEPROM_AUTO_SAVE=0)
else()
  message(FATAL_ERROR "EEPROM_AUTO_SAVE must be 0 or 1")
endif()


add_subdirectory(lib)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    src/main.c
    src/eeprom.c
    src/frequency.c
    src/gps.c
    src/int.c
    src/menu.c
)


# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    eeprom
    LCD
)
