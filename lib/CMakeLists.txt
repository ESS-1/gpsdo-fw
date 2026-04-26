add_library(eeprom)
target_sources(eeprom INTERFACE
    eeprom/ee.c
    eeprom/ee.h
)
target_include_directories(eeprom INTERFACE
    eeprom
)

target_link_libraries(eeprom stm32cubemx)
    
add_subdirectory(stm32/cmake/stm32cubemx)
