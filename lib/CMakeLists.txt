add_library(eeprom)
target_sources(eeprom INTERFACE
    eeprom/ee.c
    eeprom/ee.h
)
target_include_directories(eeprom INTERFACE
    eeprom
)

add_library(st7735)
target_sources(st7735 INTERFACE
    st7735/st7735.c
    st7735/st7735.h
    st7735/fonts.c
    st7735/fonts.h
)
target_include_directories(st7735 INTERFACE
    st7735
)

target_link_libraries(st7735 stm32cubemx)
target_link_libraries(eeprom stm32cubemx)
    
add_subdirectory(stm32/cmake/stm32cubemx)
