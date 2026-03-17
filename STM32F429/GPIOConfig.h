/* --- GPIOConfig.h --- */

/* ------------------------------------------
Author: Pnbeldor
Date: 3/12/2026
------------------------------------------ */

#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

namespace STM32F429 {

//=============================================================================
// GPIO Port Definitions
//=============================================================================
enum class GPIOPort : uint8_t {
    PORTA,
    PORTB,
    PORTC,
    PORTD,
    PORTE,
    PORTF,
    PORTG,
    PORTH,
    PORTI,
    PORTJ,
    PORTK
};

//=============================================================================
// Pin Mode Definitions
//=============================================================================
enum class PinMode : uint8_t {
    INPUT = 0x00,           // 00: Input mode
    OUTPUT = 0x01,          // 01: General purpose output mode
    ALTERNATE = 0x02,       // 10: Alternate function mode
    ANALOG = 0x03           // 11: Analog mode
};

//=============================================================================
// Output Type Definitions
//=============================================================================
enum class OutputType : uint8_t {
    PUSH_PULL = 0x00,       // 0: Output push-pull
    OPEN_DRAIN = 0x01       // 1: Output open-drain
};

//=============================================================================
// Output Speed Definitions
//=============================================================================
enum class OutputSpeed : uint8_t {
    LOW = 0x00,             // 00: Low speed (2 MHz)
    MEDIUM = 0x01,          // 01: Medium speed (10-25 MHz)
    HIGH = 0x02,            // 10: High speed (50 MHz)
    VERY_HIGH = 0x03        // 11: Very high speed (100+ MHz)
};

//=============================================================================
// Pull-up/Pull-down Definitions
//=============================================================================
enum class PullMode : uint8_t {
    NO_PULL = 0x00,         // 00: No pull-up, pull-down
    PULL_UP = 0x01,         // 01: Pull-up
    PULL_DOWN = 0x02        // 10: Pull-down
};

//=============================================================================
// Alternate Function Definitions
//=============================================================================
enum class AlternateFunction : uint8_t {
    AF0 = 0,    // System
    AF1 = 1,    // TIM1/TIM2
    AF2 = 2,    // TIM3/TIM4/TIM5
    AF3 = 3,    // TIM8/TIM9/TIM10/TIM11
    AF4 = 4,    // I2C1/I2C2/I2C3
    AF5 = 5,    // SPI1/SPI2/SPI3/SPI4/SPI5/SPI6
    AF6 = 6,    // SPI2/SPI3/SAI1
    AF7 = 7,    // SPI3/USART1/USART2/USART3
    AF8 = 8,    // USART6/UART4/UART5/UART7/UART8
    AF9 = 9,    // CAN1/CAN2/TIM12/TIM13/TIM14/LCD
    AF10 = 10,  // OTG2_HS/OTG1_FS
    AF11 = 11,  // ETH
    AF12 = 12,  // FMC/SDIO/OTG2_FS
    AF13 = 13,  // DCMI
    AF14 = 14,  // LCD
    AF15 = 15   // EVENTOUT
};

//=============================================================================
// Named Pin Definitions for STM32F429 Discovery Board
//=============================================================================
struct NamedPin {
    GPIOPort port;
    uint8_t pin;
    const char* name;
    const char* description;
    
    // Constexpr constructor
    constexpr NamedPin(GPIOPort p, uint8_t pi, const char* n, const char* d)
        : port(p), pin(pi), name(n), description(d) {}
};

class PinNames {
public:
    // User LEDs (Onboard LEDs on Discovery board)
    static constexpr NamedPin LED_GREEN = { GPIOPort::PORTG, 13, "LED_GREEN", "Onboard Green LED (PG13)" };
    static constexpr NamedPin LED_RED = { GPIOPort::PORTG, 14, "LED_RED", "Onboard Red LED (PG14)" };
    
    // User Button
    static constexpr NamedPin USER_BUTTON = { GPIOPort::PORTA, 0, "USER_BUTTON", "User button (PA0)" };
    
    // USB OTG FS
    static constexpr NamedPin USB_OTG_FS_DM = { GPIOPort::PORTA, 11, "USB_FS_DM", "USB OTG FS DM" };
    static constexpr NamedPin USB_OTG_FS_DP = { GPIOPort::PORTA, 12, "USB_FS_DP", "USB OTG FS DP" };
    static constexpr NamedPin USB_OTG_FS_VBUS = { GPIOPort::PORTA, 9, "USB_FS_VBUS", "USB OTG FS VBUS" };
    static constexpr NamedPin USB_OTG_FS_ID = { GPIOPort::PORTA, 10, "USB_FS_ID", "USB OTG FS ID" };
    
    // USB OTG HS (using ULPI)
    static constexpr NamedPin USB_HS_ULPI_CK = { GPIOPort::PORTA, 5, "USB_HS_CK", "USB OTG HS ULPI Clock" };
    static constexpr NamedPin USB_HS_ULPI_D0 = { GPIOPort::PORTA, 3, "USB_HS_D0", "USB OTG HS ULPI D0" };
    static constexpr NamedPin USB_HS_ULPI_D1 = { GPIOPort::PORTA, 6, "USB_HS_D1", "USB OTG HS ULPI D1" };
    static constexpr NamedPin USB_HS_ULPI_D2 = { GPIOPort::PORTA, 7, "USB_HS_D2", "USB OTG HS ULPI D2" };
    static constexpr NamedPin USB_HS_ULPI_D3 = { GPIOPort::PORTB, 10, "USB_HS_D3", "USB OTG HS ULPI D3" };
    static constexpr NamedPin USB_HS_ULPI_D4 = { GPIOPort::PORTB, 11, "USB_HS_D4", "USB OTG HS ULPI D4" };
    static constexpr NamedPin USB_HS_ULPI_D5 = { GPIOPort::PORTB, 12, "USB_HS_D5", "USB OTG HS ULPI D5" };
    static constexpr NamedPin USB_HS_ULPI_D6 = { GPIOPort::PORTB, 13, "USB_HS_D6", "USB OTG HS ULPI D6" };
    static constexpr NamedPin USB_HS_ULPI_D7 = { GPIOPort::PORTB, 5, "USB_HS_D7", "USB OTG HS ULPI D7" };
    static constexpr NamedPin USB_HS_ULPI_STP = { GPIOPort::PORTC, 0, "USB_HS_STP", "USB OTG HS ULPI STP" };
    static constexpr NamedPin USB_HS_ULPI_DIR = { GPIOPort::PORTC, 2, "USB_HS_DIR", "USB OTG HS ULPI DIR" };
    static constexpr NamedPin USB_HS_ULPI_NXT = { GPIOPort::PORTC, 3, "USB_HS_NXT", "USB OTG HS ULPI NXT" };
    
    // Ethernet
    static constexpr NamedPin ETH_MDC = { GPIOPort::PORTC, 1, "ETH_MDC", "Ethernet MDC" };
    static constexpr NamedPin ETH_MDIO = { GPIOPort::PORTA, 2, "ETH_MDIO", "Ethernet MDIO" };
    static constexpr NamedPin ETH_MII_RX_CLK = { GPIOPort::PORTA, 1, "ETH_RX_CLK", "Ethernet MII RX Clock" };
    static constexpr NamedPin ETH_MII_RX_DV = { GPIOPort::PORTA, 7, "ETH_RX_DV", "Ethernet MII RX Data Valid" };
    static constexpr NamedPin ETH_MII_RXD0 = { GPIOPort::PORTC, 4, "ETH_RXD0", "Ethernet MII RXD0" };
    static constexpr NamedPin ETH_MII_RXD1 = { GPIOPort::PORTC, 5, "ETH_RXD1", "Ethernet MII RXD1" };
    static constexpr NamedPin ETH_MII_RXD2 = { GPIOPort::PORTH, 6, "ETH_RXD2", "Ethernet MII RXD2" };
    static constexpr NamedPin ETH_MII_RXD3 = { GPIOPort::PORTH, 7, "ETH_RXD3", "Ethernet MII RXD3" };
    static constexpr NamedPin ETH_MII_TX_CLK = { GPIOPort::PORTC, 3, "ETH_TX_CLK", "Ethernet MII TX Clock" };
    static constexpr NamedPin ETH_MII_TX_EN = { GPIOPort::PORTB, 11, "ETH_TX_EN", "Ethernet MII TX Enable" };
    static constexpr NamedPin ETH_MII_TXD0 = { GPIOPort::PORTB, 12, "ETH_TXD0", "Ethernet MII TXD0" };
    static constexpr NamedPin ETH_MII_TXD1 = { GPIOPort::PORTB, 13, "ETH_TXD1", "Ethernet MII TXD1" };
    static constexpr NamedPin ETH_MII_TXD2 = { GPIOPort::PORTC, 2, "ETH_TXD2", "Ethernet MII TXD2" };
    static constexpr NamedPin ETH_MII_TXD3 = { GPIOPort::PORTE, 2, "ETH_TXD3", "Ethernet MII TXD3" };
    static constexpr NamedPin ETH_MII_CRS = { GPIOPort::PORTA, 0, "ETH_CRS", "Ethernet MII CRS" };
    static constexpr NamedPin ETH_MII_COL = { GPIOPort::PORTA, 3, "ETH_COL", "Ethernet MII COL" };
    
    // I2C
    static constexpr NamedPin I2C1_SCL = { GPIOPort::PORTB, 6, "I2C1_SCL", "I2C1 Clock" };
    static constexpr NamedPin I2C1_SDA = { GPIOPort::PORTB, 7, "I2C1_SDA", "I2C1 Data" };
    static constexpr NamedPin I2C1_SMBA = { GPIOPort::PORTB, 5, "I2C1_SMBA", "I2C1 SMBA" };
    static constexpr NamedPin I2C2_SCL = { GPIOPort::PORTB, 10, "I2C2_SCL", "I2C2 Clock" };
    static constexpr NamedPin I2C2_SDA = { GPIOPort::PORTB, 11, "I2C2_SDA", "I2C2 Data" };
    static constexpr NamedPin I2C2_SMBA = { GPIOPort::PORTB, 12, "I2C2_SMBA", "I2C2 SMBA" };
    static constexpr NamedPin I2C3_SCL = { GPIOPort::PORTA, 8, "I2C3_SCL", "I2C3 Clock" };
    static constexpr NamedPin I2C3_SDA = { GPIOPort::PORTC, 9, "I2C3_SDA", "I2C3 Data" };
    static constexpr NamedPin I2C3_SMBA = { GPIOPort::PORTA, 9, "I2C3_SMBA", "I2C3 SMBA" };
    
    // SPI
    static constexpr NamedPin SPI1_NSS = { GPIOPort::PORTA, 4, "SPI1_NSS", "SPI1 NSS" };
    static constexpr NamedPin SPI1_SCK = { GPIOPort::PORTA, 5, "SPI1_SCK", "SPI1 Clock" };
    static constexpr NamedPin SPI1_MISO = { GPIOPort::PORTA, 6, "SPI1_MISO", "SPI1 MISO" };
    static constexpr NamedPin SPI1_MOSI = { GPIOPort::PORTA, 7, "SPI1_MOSI", "SPI1 MOSI" };
    
    static constexpr NamedPin SPI2_NSS = { GPIOPort::PORTB, 12, "SPI2_NSS", "SPI2 NSS / I2S2 WS" };
    static constexpr NamedPin SPI2_SCK = { GPIOPort::PORTB, 13, "SPI2_SCK", "SPI2 Clock / I2S2 CK" };
    static constexpr NamedPin SPI2_MISO = { GPIOPort::PORTB, 14, "SPI2_MISO", "SPI2 MISO" };
    static constexpr NamedPin SPI2_MOSI = { GPIOPort::PORTB, 15, "SPI2_MOSI", "SPI2 MOSI / I2S2 SD" };
    
    static constexpr NamedPin SPI3_NSS = { GPIOPort::PORTA, 15, "SPI3_NSS", "SPI3 NSS / I2S3 WS" };
    static constexpr NamedPin SPI3_SCK = { GPIOPort::PORTC, 10, "SPI3_SCK", "SPI3 Clock / I2S3 CK" };
    static constexpr NamedPin SPI3_MISO = { GPIOPort::PORTC, 11, "SPI3_MISO", "SPI3 MISO" };
    static constexpr NamedPin SPI3_MOSI = { GPIOPort::PORTC, 12, "SPI3_MOSI", "SPI3 MOSI / I2S3 SD" };
    
    static constexpr NamedPin SPI4_NSS = { GPIOPort::PORTE, 11, "SPI4_NSS", "SPI4 NSS" };
    static constexpr NamedPin SPI4_SCK = { GPIOPort::PORTE, 12, "SPI4_SCK", "SPI4 Clock" };
    static constexpr NamedPin SPI4_MISO = { GPIOPort::PORTE, 13, "SPI4_MISO", "SPI4 MISO" };
    static constexpr NamedPin SPI4_MOSI = { GPIOPort::PORTE, 14, "SPI4_MOSI", "SPI4 MOSI" };
    
    static constexpr NamedPin SPI5_NSS = { GPIOPort::PORTF, 6, "SPI5_NSS", "SPI5 NSS" };
    static constexpr NamedPin SPI5_SCK = { GPIOPort::PORTF, 7, "SPI5_SCK", "SPI5 Clock" };
    static constexpr NamedPin SPI5_MISO = { GPIOPort::PORTF, 8, "SPI5_MISO", "SPI5 MISO" };
    static constexpr NamedPin SPI5_MOSI = { GPIOPort::PORTF, 9, "SPI5_MOSI", "SPI5 MOSI" };
    
    static constexpr NamedPin SPI6_NSS = { GPIOPort::PORTG, 8, "SPI6_NSS", "SPI6 NSS" };
    static constexpr NamedPin SPI6_SCK = { GPIOPort::PORTG, 13, "SPI6_SCK", "SPI6 Clock" };
    static constexpr NamedPin SPI6_MISO = { GPIOPort::PORTG, 12, "SPI6_MISO", "SPI6 MISO" };
    static constexpr NamedPin SPI6_MOSI = { GPIOPort::PORTG, 14, "SPI6_MOSI", "SPI6 MOSI" };
    
    // USART/UART
    static constexpr NamedPin USART1_TX = { GPIOPort::PORTA, 9, "USART1_TX", "USART1 Transmit" };
    static constexpr NamedPin USART1_RX = { GPIOPort::PORTA, 10, "USART1_RX", "USART1 Receive" };
    static constexpr NamedPin USART1_CK = { GPIOPort::PORTA, 8, "USART1_CK", "USART1 Clock" };
    static constexpr NamedPin USART1_CTS = { GPIOPort::PORTA, 11, "USART1_CTS", "USART1 CTS" };
    static constexpr NamedPin USART1_RTS = { GPIOPort::PORTA, 12, "USART1_RTS", "USART1 RTS" };
    
    static constexpr NamedPin USART2_TX = { GPIOPort::PORTA, 2, "USART2_TX", "USART2 Transmit" };
    static constexpr NamedPin USART2_RX = { GPIOPort::PORTA, 3, "USART2_RX", "USART2 Receive" };
    static constexpr NamedPin USART2_CK = { GPIOPort::PORTA, 4, "USART2_CK", "USART2 Clock" };
    static constexpr NamedPin USART2_CTS = { GPIOPort::PORTA, 0, "USART2_CTS", "USART2 CTS" };
    static constexpr NamedPin USART2_RTS = { GPIOPort::PORTA, 1, "USART2_RTS", "USART2 RTS" };
    
    static constexpr NamedPin USART3_TX = { GPIOPort::PORTD, 8, "USART3_TX", "USART3 Transmit" };
    static constexpr NamedPin USART3_RX = { GPIOPort::PORTD, 9, "USART3_RX", "USART3 Receive" };
    static constexpr NamedPin USART3_CK = { GPIOPort::PORTD, 10, "USART3_CK", "USART3 Clock" };
    static constexpr NamedPin USART3_CTS = { GPIOPort::PORTD, 11, "USART3_CTS", "USART3 CTS" };
    static constexpr NamedPin USART3_RTS = { GPIOPort::PORTD, 12, "USART3_RTS", "USART3 RTS" };
    
    static constexpr NamedPin UART4_TX = { GPIOPort::PORTA, 0, "UART4_TX", "UART4 Transmit" };
    static constexpr NamedPin UART4_RX = { GPIOPort::PORTA, 1, "UART4_RX", "UART4 Receive" };
    
    static constexpr NamedPin UART5_TX = { GPIOPort::PORTC, 12, "UART5_TX", "UART5 Transmit" };
    static constexpr NamedPin UART5_RX = { GPIOPort::PORTD, 2, "UART5_RX", "UART5 Receive" };
    
    static constexpr NamedPin USART6_TX = { GPIOPort::PORTC, 6, "USART6_TX", "USART6 Transmit" };
    static constexpr NamedPin USART6_RX = { GPIOPort::PORTC, 7, "USART6_RX", "USART6 Receive" };
    static constexpr NamedPin USART6_CK = { GPIOPort::PORTC, 8, "USART6_CK", "USART6 Clock" };
    static constexpr NamedPin USART6_CTS = { GPIOPort::PORTG, 13, "USART6_CTS", "USART6 CTS" };
    static constexpr NamedPin USART6_RTS = { GPIOPort::PORTG, 12, "USART6_RTS", "USART6 RTS" };
    
    static constexpr NamedPin UART7_TX = { GPIOPort::PORTF, 7, "UART7_TX", "UART7 Transmit" };
    static constexpr NamedPin UART7_RX = { GPIOPort::PORTF, 6, "UART7_RX", "UART7 Receive" };
    
    static constexpr NamedPin UART8_TX = { GPIOPort::PORTE, 1, "UART8_TX", "UART8 Transmit" };
    static constexpr NamedPin UART8_RX = { GPIOPort::PORTE, 0, "UART8_RX", "UART8 Receive" };
    
    // CAN
    static constexpr NamedPin CAN1_TX = { GPIOPort::PORTB, 9, "CAN1_TX", "CAN1 Transmit" };
    static constexpr NamedPin CAN1_RX = { GPIOPort::PORTB, 8, "CAN1_RX", "CAN1 Receive" };
    
    static constexpr NamedPin CAN2_TX = { GPIOPort::PORTB, 13, "CAN2_TX", "CAN2 Transmit" };
    static constexpr NamedPin CAN2_RX = { GPIOPort::PORTB, 12, "CAN2_RX", "CAN2 Receive" };
    
    // SDIO
    static constexpr NamedPin SDIO_CK = { GPIOPort::PORTC, 12, "SDIO_CK", "SDIO Clock" };
    static constexpr NamedPin SDIO_CMD = { GPIOPort::PORTD, 2, "SDIO_CMD", "SDIO Command" };
    static constexpr NamedPin SDIO_D0 = { GPIOPort::PORTC, 8, "SDIO_D0", "SDIO Data 0" };
    static constexpr NamedPin SDIO_D1 = { GPIOPort::PORTC, 9, "SDIO_D1", "SDIO Data 1" };
    static constexpr NamedPin SDIO_D2 = { GPIOPort::PORTC, 10, "SDIO_D2", "SDIO Data 2" };
    static constexpr NamedPin SDIO_D3 = { GPIOPort::PORTC, 11, "SDIO_D3", "SDIO Data 3" };
    static constexpr NamedPin SDIO_D4 = { GPIOPort::PORTB, 8, "SDIO_D4", "SDIO Data 4" };
    static constexpr NamedPin SDIO_D5 = { GPIOPort::PORTB, 9, "SDIO_D5", "SDIO Data 5" };
    static constexpr NamedPin SDIO_D6 = { GPIOPort::PORTC, 6, "SDIO_D6", "SDIO Data 6" };
    static constexpr NamedPin SDIO_D7 = { GPIOPort::PORTC, 7, "SDIO_D7", "SDIO Data 7" };
    
    // DCMI (Digital Camera Interface)
    static constexpr NamedPin DCMI_PIXCLK = { GPIOPort::PORTA, 6, "DCMI_PIXCLK", "DCMI Pixel Clock" };
    static constexpr NamedPin DCMI_HSYNC = { GPIOPort::PORTA, 4, "DCMI_HSYNC", "DCMI Horizontal Sync" };
    static constexpr NamedPin DCMI_VSYNC = { GPIOPort::PORTG, 9, "DCMI_VSYNC", "DCMI Vertical Sync" };
    static constexpr NamedPin DCMI_D0 = { GPIOPort::PORTH, 9, "DCMI_D0", "DCMI Data 0" };
    static constexpr NamedPin DCMI_D1 = { GPIOPort::PORTH, 10, "DCMI_D1", "DCMI Data 1" };
    static constexpr NamedPin DCMI_D2 = { GPIOPort::PORTH, 11, "DCMI_D2", "DCMI Data 2" };
    static constexpr NamedPin DCMI_D3 = { GPIOPort::PORTH, 12, "DCMI_D3", "DCMI Data 3" };
    static constexpr NamedPin DCMI_D4 = { GPIOPort::PORTH, 14, "DCMI_D4", "DCMI Data 4" };
    static constexpr NamedPin DCMI_D5 = { GPIOPort::PORTD, 3, "DCMI_D5", "DCMI Data 5" };
    static constexpr NamedPin DCMI_D6 = { GPIOPort::PORTB, 8, "DCMI_D6", "DCMI Data 6" };
    static constexpr NamedPin DCMI_D7 = { GPIOPort::PORTB, 9, "DCMI_D7", "DCMI Data 7" };
    static constexpr NamedPin DCMI_D8 = { GPIOPort::PORTH, 6, "DCMI_D8", "DCMI Data 8" };
    static constexpr NamedPin DCMI_D9 = { GPIOPort::PORTH, 7, "DCMI_D9", "DCMI Data 9" };
    static constexpr NamedPin DCMI_D10 = { GPIOPort::PORTD, 6, "DCMI_D10", "DCMI Data 10" };
    static constexpr NamedPin DCMI_D11 = { GPIOPort::PORTD, 2, "DCMI_D11", "DCMI Data 11" };
    static constexpr NamedPin DCMI_D12 = { GPIOPort::PORTG, 6, "DCMI_D12", "DCMI Data 12" };
    static constexpr NamedPin DCMI_D13 = { GPIOPort::PORTG, 15, "DCMI_D13", "DCMI Data 13" };
    
    // LCD-TFT
    static constexpr NamedPin LCD_CLK = { GPIOPort::PORTE, 14, "LCD_CLK", "LCD Clock" };
    static constexpr NamedPin LCD_HSYNC = { GPIOPort::PORTC, 6, "LCD_HSYNC", "LCD Horizontal Sync" };
    static constexpr NamedPin LCD_VSYNC = { GPIOPort::PORTA, 4, "LCD_VSYNC", "LCD Vertical Sync" };
    static constexpr NamedPin LCD_DE = { GPIOPort::PORTE, 13, "LCD_DE", "LCD Data Enable" };
    static constexpr NamedPin LCD_R0 = { GPIOPort::PORTH, 2, "LCD_R0", "LCD Red 0" };
    static constexpr NamedPin LCD_R1 = { GPIOPort::PORTH, 3, "LCD_R1", "LCD Red 1" };
    static constexpr NamedPin LCD_R2 = { GPIOPort::PORTH, 8, "LCD_R2", "LCD Red 2" };
    static constexpr NamedPin LCD_R3 = { GPIOPort::PORTH, 9, "LCD_R3", "LCD Red 3" };
    static constexpr NamedPin LCD_R4 = { GPIOPort::PORTH, 10, "LCD_R4", "LCD Red 4" };
    static constexpr NamedPin LCD_R5 = { GPIOPort::PORTH, 11, "LCD_R5", "LCD Red 5" };
    static constexpr NamedPin LCD_R6 = { GPIOPort::PORTA, 8, "LCD_R6", "LCD Red 6" };
    static constexpr NamedPin LCD_R7 = { GPIOPort::PORTE, 15, "LCD_R7", "LCD Red 7" };
    static constexpr NamedPin LCD_G0 = { GPIOPort::PORTE, 5, "LCD_G0", "LCD Green 0" };
    static constexpr NamedPin LCD_G1 = { GPIOPort::PORTE, 6, "LCD_G1", "LCD Green 1" };
    static constexpr NamedPin LCD_G2 = { GPIOPort::PORTA, 6, "LCD_G2", "LCD Green 2" };
    static constexpr NamedPin LCD_G3 = { GPIOPort::PORTE, 11, "LCD_G3", "LCD Green 3" };
    static constexpr NamedPin LCD_G4 = { GPIOPort::PORTB, 10, "LCD_G4", "LCD Green 4" };
    static constexpr NamedPin LCD_G5 = { GPIOPort::PORTB, 11, "LCD_G5", "LCD Green 5" };
    static constexpr NamedPin LCD_G6 = { GPIOPort::PORTC, 7, "LCD_G6", "LCD Green 6" };
    static constexpr NamedPin LCD_G7 = { GPIOPort::PORTD, 3, "LCD_G7", "LCD Green 7" };
    static constexpr NamedPin LCD_B0 = { GPIOPort::PORTE, 4, "LCD_B0", "LCD Blue 0" };
    static constexpr NamedPin LCD_B1 = { GPIOPort::PORTG, 12, "LCD_B1", "LCD Blue 1" };
    static constexpr NamedPin LCD_B2 = { GPIOPort::PORTD, 6, "LCD_B2", "LCD Blue 2" };
    static constexpr NamedPin LCD_B3 = { GPIOPort::PORTD, 10, "LCD_B3", "LCD Blue 3" };
    static constexpr NamedPin LCD_B4 = { GPIOPort::PORTE, 12, "LCD_B4", "LCD Blue 4" };
    static constexpr NamedPin LCD_B5 = { GPIOPort::PORTA, 3, "LCD_B5", "LCD Blue 5" };
    static constexpr NamedPin LCD_B6 = { GPIOPort::PORTB, 8, "LCD_B6", "LCD Blue 6" };
    static constexpr NamedPin LCD_B7 = { GPIOPort::PORTB, 9, "LCD_B7", "LCD Blue 7" };
    
    // SAI (Serial Audio Interface)
    static constexpr NamedPin SAI1_SCK_A = { GPIOPort::PORTE, 5, "SAI1_SCK_A", "SAI1 Clock A" };
    static constexpr NamedPin SAI1_FS_A = { GPIOPort::PORTE, 4, "SAI1_FS_A", "SAI1 Frame Sync A" };
    static constexpr NamedPin SAI1_SD_A = { GPIOPort::PORTE, 6, "SAI1_SD_A", "SAI1 Data A" };
    static constexpr NamedPin SAI1_MCLK_A = { GPIOPort::PORTE, 2, "SAI1_MCLK_A", "SAI1 Master Clock A" };
    static constexpr NamedPin SAI1_SCK_B = { GPIOPort::PORTF, 8, "SAI1_SCK_B", "SAI1 Clock B" };
    static constexpr NamedPin SAI1_FS_B = { GPIOPort::PORTF, 9, "SAI1_FS_B", "SAI1 Frame Sync B" };
    static constexpr NamedPin SAI1_SD_B = { GPIOPort::PORTF, 6, "SAI1_SD_B", "SAI1 Data B" };
    static constexpr NamedPin SAI1_MCLK_B = { GPIOPort::PORTF, 7, "SAI1_MCLK_B", "SAI1 Master Clock B" };
    
    // I2S
    static constexpr NamedPin I2S2_CK = { GPIOPort::PORTB, 13, "I2S2_CK", "I2S2 Clock" };
    static constexpr NamedPin I2S2_WS = { GPIOPort::PORTB, 12, "I2S2_WS", "I2S2 Word Select" };
    static constexpr NamedPin I2S2_SD = { GPIOPort::PORTB, 15, "I2S2_SD", "I2S2 Data" };
    static constexpr NamedPin I2S3_CK = { GPIOPort::PORTC, 10, "I2S3_CK", "I2S3 Clock" };
    static constexpr NamedPin I2S3_WS = { GPIOPort::PORTA, 15, "I2S3_WS", "I2S3 Word Select" };
    static constexpr NamedPin I2S3_SD = { GPIOPort::PORTC, 12, "I2S3_SD", "I2S3 Data" };
    
    // Timers
    static constexpr NamedPin TIM2_CH1 = { GPIOPort::PORTA, 0, "TIM2_CH1", "Timer 2 Channel 1" };
    static constexpr NamedPin TIM2_CH2 = { GPIOPort::PORTA, 1, "TIM2_CH2", "Timer 2 Channel 2" };
    static constexpr NamedPin TIM2_CH3 = { GPIOPort::PORTA, 2, "TIM2_CH3", "Timer 2 Channel 3" };
    static constexpr NamedPin TIM2_CH4 = { GPIOPort::PORTA, 3, "TIM2_CH4", "Timer 2 Channel 4" };
    
    static constexpr NamedPin TIM3_CH1 = { GPIOPort::PORTA, 6, "TIM3_CH1", "Timer 3 Channel 1" };
    static constexpr NamedPin TIM3_CH2 = { GPIOPort::PORTA, 7, "TIM3_CH2", "Timer 3 Channel 2" };
    static constexpr NamedPin TIM3_CH3 = { GPIOPort::PORTB, 0, "TIM3_CH3", "Timer 3 Channel 3" };
    static constexpr NamedPin TIM3_CH4 = { GPIOPort::PORTB, 1, "TIM3_CH4", "Timer 3 Channel 4" };
    
    static constexpr NamedPin TIM4_CH1 = { GPIOPort::PORTB, 6, "TIM4_CH1", "Timer 4 Channel 1" };
    static constexpr NamedPin TIM4_CH2 = { GPIOPort::PORTB, 7, "TIM4_CH2", "Timer 4 Channel 2" };
    static constexpr NamedPin TIM4_CH3 = { GPIOPort::PORTB, 8, "TIM4_CH3", "Timer 4 Channel 3" };
    static constexpr NamedPin TIM4_CH4 = { GPIOPort::PORTB, 9, "TIM4_CH4", "Timer 4 Channel 4" };
    
    static constexpr NamedPin TIM5_CH1 = { GPIOPort::PORTA, 0, "TIM5_CH1", "Timer 5 Channel 1" };
    static constexpr NamedPin TIM5_CH2 = { GPIOPort::PORTA, 1, "TIM5_CH2", "Timer 5 Channel 2" };
    static constexpr NamedPin TIM5_CH3 = { GPIOPort::PORTA, 2, "TIM5_CH3", "Timer 5 Channel 3" };
    static constexpr NamedPin TIM5_CH4 = { GPIOPort::PORTA, 3, "TIM5_CH4", "Timer 5 Channel 4" };
    
    static constexpr NamedPin TIM8_CH1 = { GPIOPort::PORTC, 6, "TIM8_CH1", "Timer 8 Channel 1" };
    static constexpr NamedPin TIM8_CH2 = { GPIOPort::PORTC, 7, "TIM8_CH2", "Timer 8 Channel 2" };
    static constexpr NamedPin TIM8_CH3 = { GPIOPort::PORTC, 8, "TIM8_CH3", "Timer 8 Channel 3" };
    static constexpr NamedPin TIM8_CH4 = { GPIOPort::PORTC, 9, "TIM8_CH4", "Timer 8 Channel 4" };
    
    // ADC inputs
    static constexpr NamedPin ADC1_IN0 = { GPIOPort::PORTA, 0, "ADC1_IN0", "ADC1 Channel 0" };
    static constexpr NamedPin ADC1_IN1 = { GPIOPort::PORTA, 1, "ADC1_IN1", "ADC1 Channel 1" };
    static constexpr NamedPin ADC1_IN2 = { GPIOPort::PORTA, 2, "ADC1_IN2", "ADC1 Channel 2" };
    static constexpr NamedPin ADC1_IN3 = { GPIOPort::PORTA, 3, "ADC1_IN3", "ADC1 Channel 3" };
    static constexpr NamedPin ADC1_IN4 = { GPIOPort::PORTA, 4, "ADC1_IN4", "ADC1 Channel 4" };
    static constexpr NamedPin ADC1_IN5 = { GPIOPort::PORTA, 5, "ADC1_IN5", "ADC1 Channel 5" };
    static constexpr NamedPin ADC1_IN6 = { GPIOPort::PORTA, 6, "ADC1_IN6", "ADC1 Channel 6" };
    static constexpr NamedPin ADC1_IN7 = { GPIOPort::PORTA, 7, "ADC1_IN7", "ADC1 Channel 7" };
    static constexpr NamedPin ADC1_IN8 = { GPIOPort::PORTB, 0, "ADC1_IN8", "ADC1 Channel 8" };
    static constexpr NamedPin ADC1_IN9 = { GPIOPort::PORTB, 1, "ADC1_IN9", "ADC1 Channel 9" };
    
    // DAC outputs
    static constexpr NamedPin DAC1_OUT = { GPIOPort::PORTA, 4, "DAC1_OUT", "DAC Channel 1 Output" };
    static constexpr NamedPin DAC2_OUT = { GPIOPort::PORTA, 5, "DAC2_OUT", "DAC Channel 2 Output" };
    
    // System
    static constexpr NamedPin OSC_IN = { GPIOPort::PORTH, 0, "OSC_IN", "Oscillator Input" };
    static constexpr NamedPin OSC_OUT = { GPIOPort::PORTH, 1, "OSC_OUT", "Oscillator Output" };
    static constexpr NamedPin NRST = { GPIOPort::PORTA, 14, "NRST", "Reset" };
    static constexpr NamedPin BOOT0 = { GPIOPort::PORTB, 2, "BOOT0", "Boot Mode Selection" };
    
    // JTAG/SWD
    static constexpr NamedPin JTMS_SWDIO = { GPIOPort::PORTA, 13, "JTMS_SWDIO", "JTMS/SWDIO" };
    static constexpr NamedPin JTCK_SWCLK = { GPIOPort::PORTA, 14, "JTCK_SWCLK", "JTCK/SWCLK" };
    static constexpr NamedPin JTDI = { GPIOPort::PORTA, 15, "JTDI", "JTDI" };
    static constexpr NamedPin JTDO = { GPIOPort::PORTB, 3, "JTDO", "JTDO/TRACESWO" };
    static constexpr NamedPin NJTRST = { GPIOPort::PORTB, 4, "NJTRST", "NJTRST" };
};

//=============================================================================
// GPIO Configuration Class
//=============================================================================
class GPIOConfig {
private:
    struct PinConfig {
        GPIOPort port;
        uint8_t pin;
        PinMode mode;
        OutputType outputType;
        OutputSpeed speed;
        PullMode pull;
        AlternateFunction alternate;
        bool isConfigured;
        std::string name;
        std::string description;
        
        PinConfig() : port(GPIOPort::PORTA), pin(0), mode(PinMode::INPUT), 
                      outputType(OutputType::PUSH_PULL), speed(OutputSpeed::LOW),
                      pull(PullMode::NO_PULL), alternate(AlternateFunction::AF0),
                      isConfigured(false), name(""), description("") {}
    };
    
    std::map<std::string, PinConfig> pinConfigs;
    
    // Helper to get port base address
    uint32_t getPortBase(GPIOPort port) const;
    
    // Helper to get port name string
    std::string getPortName(GPIOPort port) const;
    
public:
    GPIOConfig();
    
    // Pin configuration methods
    void configurePin(const NamedPin& pin, PinMode mode, 
                      OutputType outputType = OutputType::PUSH_PULL,
                      OutputSpeed speed = OutputSpeed::LOW,
                      PullMode pull = PullMode::NO_PULL,
                      AlternateFunction alt = AlternateFunction::AF0);
    
    void setInput(const NamedPin& pin, PullMode pull = PullMode::NO_PULL);
    void setOutput(const NamedPin& pin, OutputType outputType = OutputType::PUSH_PULL,
                   OutputSpeed speed = OutputSpeed::LOW);
    void setAlternate(const NamedPin& pin, AlternateFunction alt,
                      OutputType outputType = OutputType::PUSH_PULL,
                      OutputSpeed speed = OutputSpeed::VERY_HIGH,
                      PullMode pull = PullMode::NO_PULL);
    void setAnalog(const NamedPin& pin);
    
    // Pin control methods
    void setPin(const NamedPin& pin, bool state);
    bool getPin(const NamedPin& pin) const;
    void togglePin(const NamedPin& pin);
    
    // Group configuration
    void configurePins(const std::vector<NamedPin>& pins, PinMode mode,
                       OutputType outputType = OutputType::PUSH_PULL,
                       OutputSpeed speed = OutputSpeed::LOW,
                       PullMode pull = PullMode::NO_PULL,
                       AlternateFunction alt = AlternateFunction::AF0);
    
    // Display configuration
    void printConfig(const NamedPin& pin) const;
    void printAllConfigs() const;
    void printPortConfig(GPIOPort port) const;
    
    // Check if pin is configured
    bool isConfigured(const NamedPin& pin) const;
    
    // Get configuration
    PinConfig getConfig(const NamedPin& pin) const;
    
    // Clear configuration
    void clearConfig(const NamedPin& pin);
    void clearAllConfigs();
    
    // Save/Load configuration (for simulation)
    std::map<std::string, PinConfig> exportConfig() const;
    void importConfig(const std::map<std::string, PinConfig>& config);
};

} // namespace STM32F429

#endif // __GPIO_CONFIG_H__
