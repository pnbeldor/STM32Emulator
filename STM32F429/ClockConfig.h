/* --- ClockConfig.h --- */

/* ------------------------------------------
Author: Pnbeldor
Date: 3/12/2026
------------------------------------------ */

#ifndef __CLOCK_CONFIG_H__
#define __CLOCK_CONFIG_H__


#include <cstdint>
#include <iostream>
#include <string>
#include <map>

namespace STM32F429 {

//=============================================================================
// Clock Sources
//=============================================================================
enum class ClockSource {
    HSI,        // Internal 16 MHz RC oscillator
    HSE,        // External oscillator (4-26 MHz)
    PLL         // Phase-Locked Loop
};

//=============================================================================
// PLL Sources
//=============================================================================
enum class PLLSource {
    HSI,        // PLL input from HSI/2
    HSE         // PLL input from HSE
};

//=============================================================================
// System Clock Sources
//=============================================================================
enum class SystemClockSource {
    HSI,
    HSE,
    PLL
};

//=============================================================================
// AHB Prescaler
//=============================================================================
enum class AHBPrescaler {
    DIV_1 = 0x00,   // System clock not divided
    DIV_2 = 0x80,   // System clock divided by 2
    DIV_4 = 0x81,   // System clock divided by 4
    DIV_8 = 0x82,   // System clock divided by 8
    DIV_16 = 0x83,  // System clock divided by 16
    DIV_64 = 0x84,  // System clock divided by 64
    DIV_128 = 0x85, // System clock divided by 128
    DIV_256 = 0x86, // System clock divided by 256
    DIV_512 = 0x87  // System clock divided by 512
};

//=============================================================================
// APB Prescaler
//=============================================================================
enum class APBPrescaler {
    DIV_1 = 0x00,   // AHB clock not divided
    DIV_2 = 0x04,   // AHB clock divided by 2
    DIV_4 = 0x05,   // AHB clock divided by 4
    DIV_8 = 0x06,   // AHB clock divided by 8
    DIV_16 = 0x07   // AHB clock divided by 16
};

//=============================================================================
// PLL Configuration Structure
//=============================================================================
struct PLLConfig {
    PLLSource source;           // PLL input source
    uint8_t m;                  // PLL division factor (2-63)
    uint16_t n;                 // PLL multiplication factor (50-432)
    uint8_t p;                  // Main PLL division factor for system clock (2,4,6,8)
    uint8_t q;                  // PLL division factor for USB OTG FS/SDIO/RNG (2-15)
    
    // Audio PLL (PLLI2S) configuration
    uint16_t i2s_n;             // PLLI2S multiplication factor (50-432)
    uint8_t i2s_r;              // PLLI2S division factor for I2S/SAI (2-7)
    
    // LCD/Audio PLL (PLLSAI) configuration
    uint16_t sai_n;             // PLLSAI multiplication factor (50-432)
    uint8_t sai_p;              // PLLSAI division factor for LCD (2,4,6,8)
    uint8_t sai_q;              // PLLSAI division factor for SAI (2-15)
    uint8_t sai_r;              // PLLSAI division factor for R (2-7)
};

//=============================================================================
// Clock Configuration Class
//=============================================================================
class ClockConfig {
private:
    // Clock frequencies (in Hz)
    uint32_t hsiFrequency;      // 16 MHz
    uint32_t hseFrequency;      // External oscillator frequency
    uint32_t sysclk;            // System clock frequency
    uint32_t hclk;              // AHB bus clock frequency
    uint32_t pclk1;             // APB1 bus clock frequency (max 45 MHz)
    uint32_t pclk2;             // APB2 bus clock frequency (max 90 MHz)
    
    // Current configuration
    ClockSource currentSource;
    PLLConfig pllConfig;
    AHBPrescaler ahbPrescaler;
    APBPrescaler apb1Prescaler;
    APBPrescaler apb2Prescaler;
    
    // Over-drive mode
    bool overdriveEnabled;
    
    // Helper methods
    uint32_t calculatePLLFrequency() const;
    void updateBusFrequencies();
    
public:
    ClockConfig();
    
    // Basic clock configuration
    void setHSECrystal(uint32_t frequencyHz);
    void useHSI();
    void useHSE();
    void usePLL(const PLLConfig& config);
    
    // Prescaler configuration
    void setAHBPrescaler(AHBPrescaler prescaler);
    void setAPB1Prescaler(APBPrescaler prescaler);
    void setAPB2Prescaler(APBPrescaler prescaler);
    
    // Over-drive configuration
    void enableOverdrive(bool enable);
    bool isOverdriveEnabled() const { return overdriveEnabled; }
    
    // Get current frequencies
    uint32_t getSysClk() const { return sysclk; }
    uint32_t getHCLK() const { return hclk; }
    uint32_t getPCLK1() const { return pclk1; }
    uint32_t getPCLK2() const { return pclk2; }
    uint32_t getPLL48Clk() const;   // 48 MHz clock for USB/SDIO/RNG
    uint32_t getLCDClk() const;      // LCD-TFT clock
    
    // Predefined configurations
    static PLLConfig get168MHzConfig(uint32_t hseFreq = 8000000);
    static PLLConfig get180MHzConfig(uint32_t hseFreq = 8000000);
    static PLLConfig get120MHzConfig(uint32_t hseFreq = 8000000);
    
    // Peripheral clock enable/disable
    void enablePeripheralClock(const std::string& peripheral);
    void disablePeripheralClock(const std::string& peripheral);
    bool isPeripheralClockEnabled(const std::string& peripheral) const;
    
    // Display configuration
    void printConfig();
    
    // Validation
    bool validateConfiguration();
    std::string getLastError() const { return lastError; }
    
private:
    std::map<std::string, bool> peripheralClocks;
    std::string lastError;
};

} // namespace STM32F429

#endif // CLOCK_CONFIG_H