/* --- SPIConfig.cpp --- */

/* ------------------------------------------
author: Pnbeldor
date: 3/12/2026
------------------------------------------ */

#include "SPIConfig.h"

#include <algorithm>
#include <thread>

namespace STM32F429 {

//=============================================================================
// SPIPeripheral Implementation
//=============================================================================

SPIPeripheral::SPIPeripheral(SPIInstance inst)
    : instance(inst),
      initialized(false),
      enabled(false),
      pclk1(45000000),
      pclk2(90000000),
      dmaTxEnabled(false),
      dmaRxEnabled(false),
      dmaTxStream(0),
      dmaRxStream(0),
      bytesTransmitted(0),
      bytesReceived(0),
      errorCount(0) {
    
    // Initialize config with defaults
    config.instance = inst;
}

uint32_t SPIPeripheral::getBaseAddress() const {
    switch(instance) {
        case SPIInstance::SPI1: return 0x40013000;
        case SPIInstance::SPI2: return 0x40003800;
        case SPIInstance::SPI3: return 0x40003C00;
        case SPIInstance::SPI4: return 0x40013400;
        case SPIInstance::SPI5: return 0x40015000;
        case SPIInstance::SPI6: return 0x40015400;
        default: return 0;
    }
}

std::string SPIPeripheral::getInstanceName() const {
    switch(instance) {
        case SPIInstance::SPI1: return "SPI1";
        case SPIInstance::SPI2: return "SPI2";
        case SPIInstance::SPI3: return "SPI3";
        case SPIInstance::SPI4: return "SPI4";
        case SPIInstance::SPI5: return "SPI5";
        case SPIInstance::SPI6: return "SPI6";
        default: return "Unknown";
    }
}

uint32_t SPIPeripheral::calculateBaudRate() const {
    uint32_t pclk = (instance == SPIInstance::SPI2 || instance == SPIInstance::SPI3) ? pclk1 : pclk2;
    
    switch(config.baudRate) {
        case SPIBaudRatePrescaler::DIV_2:   return pclk / 2;
        case SPIBaudRatePrescaler::DIV_4:   return pclk / 4;
        case SPIBaudRatePrescaler::DIV_8:   return pclk / 8;
        case SPIBaudRatePrescaler::DIV_16:  return pclk / 16;
        case SPIBaudRatePrescaler::DIV_32:  return pclk / 32;
        case SPIBaudRatePrescaler::DIV_64:  return pclk / 64;
        case SPIBaudRatePrescaler::DIV_128: return pclk / 128;
        case SPIBaudRatePrescaler::DIV_256: return pclk / 256;
        default: return pclk / 8;
    }
}

bool SPIPeripheral::init(const SPIConfig& cfg) {
    config = cfg;
    initialized = true;
    
    std::cout << "Initializing " << getInstanceName() << std::endl;
    std::cout << "  Mode: " << (config.mode == SPIMode::MASTER ? "MASTER" : "SLAVE") << std::endl;
    std::cout << "  Baud Rate: " << calculateBaudRate() / 1000 << " kHz" << std::endl;
    std::cout << "  Data Size: " << ((int)config.dataSize + 1) << " bits" << std::endl;
    
    return true;
}

void SPIPeripheral::setClockFrequencies(uint32_t apb1, uint32_t apb2) {
    pclk1 = apb1;
    pclk2 = apb2;
}

void SPIPeripheral::deinit() {
    disable();
    initialized = false;
    std::cout << "Deinitialized " << getInstanceName() << std::endl;
}

bool SPIPeripheral::enable() {
    if (!initialized) {
        std::cerr << "Error: " << getInstanceName() << " not initialized" << std::endl;
        return false;
    }
    
    enabled = true;
    std::cout << "Enabled " << getInstanceName() << std::endl;
    return true;
}

void SPIPeripheral::disable() {
    enabled = false;
    std::cout << "Disabled " << getInstanceName() << std::endl;
}

bool SPIPeripheral::transmit(uint8_t data) {
    if (!enabled) {
        errorCount++;
        return false;
    }
    
    txBuffer.push(data);
    bytesTransmitted++;
    
    // Simulate SPI transmission
    if (rxBuffer.empty()) {
        // Loopback for testing
        rxBuffer.push(data);
    }
    
    return true;
}

bool SPIPeripheral::transmit(const uint8_t* data, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        if (!transmit(data[i])) {
            return false;
        }
    }
    return true;
}

bool SPIPeripheral::transmit(const std::vector<uint8_t>& data) {
    return transmit(data.data(), data.size());
}

bool SPIPeripheral::receive(uint8_t& data) {
    if (!enabled || rxBuffer.empty()) {
        errorCount++;
        return false;
    }
    
    data = rxBuffer.front();
    rxBuffer.pop();
    bytesReceived++;
    return true;
}

uint32_t SPIPeripheral::receive(uint8_t* buffer, uint32_t maxLength) {
    uint32_t count = 0;
    while (count < maxLength && !rxBuffer.empty()) {
        buffer[count++] = rxBuffer.front();
        rxBuffer.pop();
        bytesReceived++;
    }
    return count;
}

std::vector<uint8_t> SPIPeripheral::receive(uint32_t length) {
    std::vector<uint8_t> result;
    uint8_t data;
    
    for (uint32_t i = 0; i < length; i++) {
        if (receive(data)) {
            result.push_back(data);
        } else {
            break;
        }
    }
    
    return result;
}

bool SPIPeripheral::transmitReceive(uint8_t txData, uint8_t& rxData) {
    if (!transmit(txData)) {
        return false;
    }
    
    // Simulate full-duplex communication
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    
    return receive(rxData);
}

bool SPIPeripheral::transmitReceive(const uint8_t* txData, uint8_t* rxData, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        if (!transmitReceive(txData[i], rxData[i])) {
            return false;
        }
    }
    return true;
}

void SPIPeripheral::enableDMA(uint8_t txStream, uint8_t rxStream) {
    dmaTxEnabled = true;
    dmaRxEnabled = true;
    dmaTxStream = txStream;
    dmaRxStream = rxStream;
    config.enableDMA = true;
    std::cout << getInstanceName() << " DMA enabled (TX Stream " 
              << (int)txStream << ", RX Stream " << (int)rxStream << ")" << std::endl;
}

void SPIPeripheral::disableDMA() {
    dmaTxEnabled = false;
    dmaRxEnabled = false;
    config.enableDMA = false;
    std::cout << getInstanceName() << " DMA disabled" << std::endl;
}

bool SPIPeripheral::startDMATransmit(const uint8_t* data, uint32_t length) {
    if (!dmaTxEnabled) {
        std::cerr << "Error: DMA TX not enabled for " << getInstanceName() << std::endl;
        return false;
    }
    
    std::cout << getInstanceName() << " DMA transmit: " << length << " bytes" << std::endl;
    return transmit(data, length);
}

bool SPIPeripheral::startDMAReceive(uint8_t* buffer, uint32_t length) {
    if (!dmaRxEnabled) {
        std::cerr << "Error: DMA RX not enabled for " << getInstanceName() << std::endl;
        return false;
    }
    
    std::cout << getInstanceName() << " DMA receive: " << length << " bytes" << std::endl;
    return receive(buffer, length) == length;
}

bool SPIPeripheral::isBusy() const {
    return !txBuffer.empty();
}

bool SPIPeripheral::isTxEmpty() const {
    return txBuffer.empty();
}

bool SPIPeripheral::isRxNotEmpty() const {
    return !rxBuffer.empty();
}

void SPIPeripheral::onTransmitComplete(std::function<void()> callback) {
    txCompleteCallback = callback;
}

void SPIPeripheral::onReceiveComplete(std::function<void()> callback) {
    rxCompleteCallback = callback;
}

void SPIPeripheral::onDataReceived(std::function<void(uint8_t)> callback) {
    dataReceivedCallback = callback;
}

void SPIPeripheral::resetStatistics() {
    bytesTransmitted = 0;
    bytesReceived = 0;
    errorCount = 0;
}

bool SPIPeripheral::writeRegister(uint8_t reg, uint8_t value) {
    return transmitReceive(reg, value);
}

bool SPIPeripheral::readRegister(uint8_t reg, uint8_t& value) {
    uint8_t dummy;
    // Send register address, then read response
    if (!transmitReceive(reg, dummy)) {
        return false;
    }
    return receive(value);
}

bool SPIPeripheral::writeReadRegister(uint8_t reg, uint8_t txValue, uint8_t& rxValue) {
    uint8_t temp;
    if (!transmitReceive(reg, temp)) {
        return false;
    }
    return transmitReceive(txValue, rxValue);
}

void SPIPeripheral::lcdWriteCommand(uint8_t cmd) {
    // LCD typically has D/C pin to distinguish command vs data
    // Here we're simulating the command write
    std::cout << getInstanceName() << " LCD Command: 0x" << std::hex << (int)cmd << std::dec << std::endl;
    transmit(cmd);
}

void SPIPeripheral::lcdWriteData(uint8_t data) {
    transmit(data);
}

void SPIPeripheral::lcdWriteData(const uint8_t* data, uint32_t length) {
    transmit(data, length);
}

void SPIPeripheral::lcdSetAddress(uint16_t x, uint16_t y) {
    // Typical LCD column/row address set
    lcdWriteCommand(0x2A); // Column address set
    lcdWriteData(x >> 8);
    lcdWriteData(x & 0xFF);
    lcdWriteData(x >> 8);
    lcdWriteData(x & 0xFF);
    
    lcdWriteCommand(0x2B); // Row address set
    lcdWriteData(y >> 8);
    lcdWriteData(y & 0xFF);
    lcdWriteData(y >> 8);
    lcdWriteData(y & 0xFF);
}

void SPIPeripheral::lcdFillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    lcdSetAddress(x, y);
    lcdWriteCommand(0x2C); // Memory write
    
    uint32_t pixels = width * height;
    uint8_t colorHigh = color >> 8;
    uint8_t colorLow = color & 0xFF;
    
    for (uint32_t i = 0; i < pixels; i++) {
        lcdWriteData(colorHigh);
        lcdWriteData(colorLow);
    }
    
    std::cout << getInstanceName() << " LCD: Filled rect " << width << "x" << height 
              << " at (" << x << "," << y << ")" << std::endl;
}

void SPIPeripheral::writeRegister(uint32_t reg, uint32_t value) {
    // Simulate hardware register write
    // In real implementation, this would write to memory-mapped register
}

uint32_t SPIPeripheral::readRegister(uint32_t reg) {
    // Simulate hardware register read
    return 0;
}

void SPIPeripheral::printConfig() const {
    std::cout << "\n=== " << getInstanceName() << " Configuration ===" << std::endl;
    std::cout << "Base Address: 0x" << std::hex << getBaseAddress() << std::dec << std::endl;
    std::cout << "Mode: " << (config.mode == SPIMode::MASTER ? "MASTER" : "SLAVE") << std::endl;
    std::cout << "Baud Rate: " << calculateBaudRate() / 1000 << " kHz" << std::endl;
    std::cout << "Clock Polarity: " << (config.clockPolarity == SPIClockPolarity::LOW ? "LOW" : "HIGH") << std::endl;
    std::cout << "Clock Phase: " << (config.clockPhase == SPIClockPhase::FIRST_EDGE ? "FIRST_EDGE" : "SECOND_EDGE") << std::endl;
    std::cout << "Data Size: " << ((int)config.dataSize + 1) << " bits" << std::endl;
    std::cout << "Frame Format: " << (config.frameFormat == SPIFrameFormat::MSB_FIRST ? "MSB First" : "LSB First") << std::endl;
    std::cout << "NSS Mode: " << (config.nssMode == SPINSSMode::SOFTWARE ? "SOFTWARE" : "HARDWARE") << std::endl;
    std::cout << "CRC: " << (config.enableCRC ? "Enabled" : "Disabled") << std::endl;
    std::cout << "DMA: " << (config.enableDMA ? "Enabled" : "Disabled") << std::endl;
    std::cout << "Statistics: TX=" << bytesTransmitted << ", RX=" << bytesReceived << ", Errors=" << errorCount << std::endl;
}

} // namespace STM32F429
