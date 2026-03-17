cmake_minimum_required(VERSION 4.1.0)
project(STM32Emulator VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#add_executable(STM32Emulator main.cpp)
#add_executable(STM32Emulator STM32F429Emulator/STM32F429EmulatorAllPeripherals.cpp)
add_executable(STM32 STM32F429/GPIOConfig.cpp 
                STM32F429/ClockConfig.cpp
                STM32F429/DMAConfig.cpp
                STM32F429/DMAConfig.cpp
                STM32F429/I2CConfig.cpp
                STM32F429/SPIConfig.cpp
                STM32F429/CANConfig.cpp
                STM32F429/ClockConfig.cpp
                STM32F429/UARTConfig.cpp
                STM32F429/TimerConfig.cpp
                STM32F429/main.cpp)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
