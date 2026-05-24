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
    ../src
)

add_library(si5351)
target_sources(si5351 INTERFACE
    si5351/si5351.c
    si5351/si5351.h
)
target_include_directories(si5351 INTERFACE
    si5351
    ../src
)

target_link_libraries(eeprom stm32cubemx)
target_link_libraries(st7735 stm32cubemx)
target_link_libraries(si5351 stm32cubemx)

add_subdirectory(stm32/cmake/stm32cubemx)
