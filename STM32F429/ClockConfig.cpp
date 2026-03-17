/* --- ClockConfig.cpp --- */

/* ------------------------------------------
author: Pnbeldor
date: 3/12/2026
------------------------------------------ */

#include "ClockConfig.h"

#include <cmath>
#include <iomanip>
#include <vector>

namespace STM32F429 {

//=============================================================================
// ClockConfig Implementation
//=============================================================================

ClockConfig::ClockConfig() 
    : hsiFrequency(16000000),
      hseFrequency(8000000),
      sysclk(16000000),
      hclk(16000000),
      pclk1(16000000),
      pclk2(16000000),
      currentSource(ClockSource::HSI),
      ahbPrescaler(AHBPrescaler::DIV_1),
      apb1Prescaler(APBPrescaler::DIV_1),
      apb2Prescaler(APBPrescaler::DIV_1),
      overdriveEnabled(false) {
    
    // Initialize PLL config with defaults
    pllConfig.source = PLLSource::HSE;
    pllConfig.m = 8;
    pllConfig.n = 336;
    pllConfig.p = 2;
    pllConfig.q = 7;
    pllConfig.i2s_n = 0;
    pllConfig.sai_n = 0;
    pllConfig.sai_p = 2;
    pllConfig.sai_q = 7;
    pllConfig.sai_r = 2;
    
    // Initialize peripheral clocks
    std::vector<std::string> peripherals = {
        "GPIOA", "GPIOB", "GPIOC", "GPIOD", "GPIOE", "GPIOF", "GPIOG", "GPIOH", "GPIOI", "GPIOJ", "GPIOK",
        "DMA1", "DMA2", "DMA2D",
        "CRC", "BKPSRAM",
        "OTG_FS", "OTG_HS", "DCMI", "RNG",
        "FMC", "SDIO",
        "TIM2", "TIM3", "TIM4", "TIM5", "TIM6", "TIM7", "TIM12", "TIM13", "TIM14",
        "USART2", "USART3", "UART4", "UART5", "UART7", "UART8", "I2C1", "I2C2", "I2C3",
        "SPI2", "SPI3", "SPI4", "SPI5", "SPI6", "CAN1", "CAN2", "DAC", "WWDG",
        "TIM1", "TIM8", "TIM9", "TIM10", "TIM11", "USART1", "USART6", "SPI1",
        "LCD_TFT", "SAI1"
    };
    
    for (const auto& peri : peripherals) {
        peripheralClocks[peri] = false;
    }
}

void ClockConfig::setHSECrystal(uint32_t frequencyHz) {
    hseFrequency = frequencyHz;
    std::cout << "HSE Crystal set to " << frequencyHz/1000000 << " MHz" << std::endl;
}

void ClockConfig::useHSI() {
    currentSource = ClockSource::HSI;
    sysclk = hsiFrequency;
    updateBusFrequencies();
    std::cout << "Using HSI clock source (" << sysclk/1000000 << " MHz)" << std::endl;
}

void ClockConfig::useHSE() {
    currentSource = ClockSource::HSE;
    sysclk = hseFrequency;
    updateBusFrequencies();
    std::cout << "Using HSE clock source (" << sysclk/1000000 << " MHz)" << std::endl;
}

void ClockConfig::usePLL(const PLLConfig& config) {
    pllConfig = config;
    currentSource = ClockSource::PLL;
    
    uint32_t pllFreq = calculatePLLFrequency();
    
    // Check maximum limits
    if (pllFreq > 180000000) {
        std::cerr << "Warning: PLL frequency " << pllFreq/1000000 
                  << " MHz exceeds maximum (180 MHz)" << std::endl;
    }
    
    sysclk = pllFreq;
    updateBusFrequencies();
    
    std::cout << "Using PLL clock source (" << sysclk/1000000 << " MHz)" << std::endl;
}

uint32_t ClockConfig::calculatePLLFrequency() const {
    uint32_t sourceFreq;
    
    if (pllConfig.source == PLLSource::HSI) {
        sourceFreq = hsiFrequency / 2;  // HSI is divided by 2 for PLL input
    } else {
        sourceFreq = hseFrequency;
    }
    
    // VCO frequency = sourceFreq / M * N
    uint32_t vcoFreq = (sourceFreq / pllConfig.m) * pllConfig.n;
    
    // System clock = VCO / P
    uint32_t pllFreq = vcoFreq / pllConfig.p;
    
    return pllFreq;
}

void ClockConfig::updateBusFrequencies() {
    // Calculate HCLK (AHB clock)
    uint32_t ahbDiv;
    switch(ahbPrescaler) {
        case AHBPrescaler::DIV_1:   ahbDiv = 1; break;
        case AHBPrescaler::DIV_2:   ahbDiv = 2; break;
        case AHBPrescaler::DIV_4:   ahbDiv = 4; break;
        case AHBPrescaler::DIV_8:   ahbDiv = 8; break;
        case AHBPrescaler::DIV_16:  ahbDiv = 16; break;
        case AHBPrescaler::DIV_64:  ahbDiv = 64; break;
        case AHBPrescaler::DIV_128: ahbDiv = 128; break;
        case AHBPrescaler::DIV_256: ahbDiv = 256; break;
        case AHBPrescaler::DIV_512: ahbDiv = 512; break;
        default: ahbDiv = 1;
    }
    hclk = sysclk / ahbDiv;
    
    // Calculate PCLK1 (APB1 clock)
    uint32_t apb1Div;
    switch(apb1Prescaler) {
        case APBPrescaler::DIV_1:  apb1Div = 1; break;
        case APBPrescaler::DIV_2:  apb1Div = 2; break;
        case APBPrescaler::DIV_4:  apb1Div = 4; break;
        case APBPrescaler::DIV_8:  apb1Div = 8; break;
        case APBPrescaler::DIV_16: apb1Div = 16; break;
        default: apb1Div = 1;
    }
    pclk1 = hclk / apb1Div;
    
    // Calculate PCLK2 (APB2 clock)
    uint32_t apb2Div;
    switch(apb2Prescaler) {
        case APBPrescaler::DIV_1:  apb2Div = 1; break;
        case APBPrescaler::DIV_2:  apb2Div = 2; break;
        case APBPrescaler::DIV_4:  apb2Div = 4; break;
        case APBPrescaler::DIV_8:  apb2Div = 8; break;
        case APBPrescaler::DIV_16: apb2Div = 16; break;
        default: apb2Div = 1;
    }
    pclk2 = hclk / apb2Div;
}

void ClockConfig::setAHBPrescaler(AHBPrescaler prescaler) {
    ahbPrescaler = prescaler;
    updateBusFrequencies();
}

void ClockConfig::setAPB1Prescaler(APBPrescaler prescaler) {
    apb1Prescaler = prescaler;
    updateBusFrequencies();
}

void ClockConfig::setAPB2Prescaler(APBPrescaler prescaler) {
    apb2Prescaler = prescaler;
    updateBusFrequencies();
}

void ClockConfig::enableOverdrive(bool enable) {
    overdriveEnabled = enable;
    if (enable && sysclk > 168000000) {
        std::cout << "Overdrive enabled for " << sysclk/1000000 << " MHz operation" << std::endl;
    }
}

uint32_t ClockConfig::getPLL48Clk() const {
    if (currentSource != ClockSource::PLL) return 0;
    
    uint32_t sourceFreq = (pllConfig.source == PLLSource::HSI) ? 
                          (hsiFrequency / 2) : hseFrequency;
    
    uint32_t vcoFreq = (sourceFreq / pllConfig.m) * pllConfig.n;
    uint32_t pll48Freq = vcoFreq / pllConfig.q;
    
    return pll48Freq;
}

uint32_t ClockConfig::getLCDClk() const {
    if (pllConfig.sai_n == 0) return 0;
    
    uint32_t sourceFreq = (pllConfig.source == PLLSource::HSI) ? 
                          (hsiFrequency / 2) : hseFrequency;
    
    uint32_t vcoFreq = (sourceFreq / pllConfig.m) * pllConfig.sai_n;
    uint32_t lcdFreq = vcoFreq / pllConfig.sai_p;
    
    return lcdFreq;
}

PLLConfig ClockConfig::get168MHzConfig(uint32_t hseFreq) {
    PLLConfig config;
    config.source = PLLSource::HSE;
    config.m = hseFreq / 1000000;  // Typically 8 for 8 MHz crystal
    config.n = 336;
    config.p = 2;   // 168 MHz output
    config.q = 7;   // 48 MHz for USB
    config.i2s_n = 336;  // For audio
    config.sai_n = 336;  // For LCD/audio
    config.sai_p = 2;    // For LCD
    config.sai_q = 7;    // For SAI
    config.sai_r = 2;    // For R
    return config;
}

PLLConfig ClockConfig::get180MHzConfig(uint32_t hseFreq) {
    PLLConfig config;
    config.source = PLLSource::HSE;
    config.m = hseFreq / 1000000;  // Typically 8 for 8 MHz crystal
    config.n = 360;
    config.p = 2;   // 180 MHz output
    config.q = 7;   // 48 MHz for USB (requires overdrive)
    config.i2s_n = 360;
    config.sai_n = 360;
    config.sai_p = 2;
    config.sai_q = 7;
    config.sai_r = 2;
    return config;
}

PLLConfig ClockConfig::get120MHzConfig(uint32_t hseFreq) {
    PLLConfig config;
    config.source = PLLSource::HSE;
    config.m = hseFreq / 1000000;  // Typically 8 for 8 MHz crystal
    config.n = 240;
    config.p = 2;   // 120 MHz output
    config.q = 5;   // 48 MHz for USB
    config.i2s_n = 240;
    config.sai_n = 240;
    config.sai_p = 2;
    config.sai_q = 5;
    config.sai_r = 2;
    return config;
}

void ClockConfig::enablePeripheralClock(const std::string& peripheral) {
    auto it = peripheralClocks.find(peripheral);
    if (it != peripheralClocks.end()) {
        it->second = true;
        std::cout << "Enabled clock for " << peripheral << std::endl;
    } else {
        std::cerr << "Unknown peripheral: " << peripheral << std::endl;
    }
}

void ClockConfig::disablePeripheralClock(const std::string& peripheral) {
    auto it = peripheralClocks.find(peripheral);
    if (it != peripheralClocks.end()) {
        it->second = false;
        std::cout << "Disabled clock for " << peripheral << std::endl;
    }
}

bool ClockConfig::isPeripheralClockEnabled(const std::string& peripheral) const {
    auto it = peripheralClocks.find(peripheral);
    if (it != peripheralClocks.end()) {
        return it->second;
    }
    return false;
}

bool ClockConfig::validateConfiguration() {
    // Check APB1 frequency (max 45 MHz)
    if (pclk1 > 45000000) {
        lastError = "APB1 frequency exceeds maximum (45 MHz)";
        return false;
    }
    
    // Check APB2 frequency (max 90 MHz)
    if (pclk2 > 90000000) {
        lastError = "APB2 frequency exceeds maximum (90 MHz)";
        return false;
    }
    
    // Check HCLK frequency (max 180 MHz)
    if (hclk > 180000000) {
        lastError = "HCLK frequency exceeds maximum (180 MHz)";
        return false;
    }
    
    // Check PLL48 clock for USB (must be 48 MHz)
    if (isPeripheralClockEnabled("OTG_FS") || isPeripheralClockEnabled("OTG_HS")) {
        uint32_t pll48 = getPLL48Clk();
        if (pll48 < 47500000 || pll48 > 48500000) {
            lastError = "PLL48 clock not within 48 MHz ±1% for USB";
            return false;
        }
    }
    
    return true;
}

void ClockConfig::printConfig() {
    std::cout << "\n=== Clock Configuration ===" << std::endl;
    std::cout << "System Clock (SYSCLK): " << sysclk/1000000 << " MHz" << std::endl;
    std::cout << "AHB Clock (HCLK):      " << hclk/1000000 << " MHz" << std::endl;
    std::cout << "APB1 Clock (PCLK1):    " << pclk1/1000000 << " MHz" << std::endl;
    std::cout << "APB2 Clock (PCLK2):    " << pclk2/1000000 << " MHz" << std::endl;
    
    if (currentSource == ClockSource::PLL) {
        std::cout << "PLL48 Clock:            " << getPLL48Clk()/1000000 << " MHz" << std::endl;
        std::cout << "LCD Clock:              " << getLCDClk()/1000000 << " MHz" << std::endl;
    }
    
    std::cout << "Overdrive:              " << (overdriveEnabled ? "Enabled" : "Disabled") << std::endl;
    
    // Print peripheral clock status
    std::cout << "\nEnabled Peripherals:" << std::endl;
    int count = 0;
    for (const auto& peri : peripheralClocks) {
        if (peri.second) {
            std::cout << "  " << peri.first;
            if (++count % 4 == 0) std::cout << std::endl;
        }
    }
    if (count % 4 != 0) std::cout << std::endl;

    // Print validation status
    std::cout << "\nConfiguration: " 
              << (validateConfiguration() ? "VALID" : "INVALID") << std::endl;
}

} // namespace STM32F429