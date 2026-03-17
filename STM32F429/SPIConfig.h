/* --- SPIConfig.h --- */

/* ------------------------------------------
Author: Pnbeldor
Date: 3/12/2026
------------------------------------------ */

#ifndef __SPI_CONFIG_H__
#define __SPI_CONFIG_H__

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <queue>

namespace STM32F429 {

//=============================================================================
// SPI Instance Selection
//=============================================================================
enum class SPIInstance {
    SPI1,   // APB2 max 90 MHz
    SPI2,   // APB1 max 45 MHz
    SPI3,   // APB1 max 45 MHz
    SPI4,   // APB2 max 90 MHz
    SPI5,   // APB2 max 90 MHz
    SPI6    // APB2 max 90 MHz
};

//=============================================================================
// SPI Mode
//=============================================================================
enum class SPIMode {
    SLAVE = 0,
    MASTER = 1
};

//=============================================================================
// SPI Baud Rate Prescaler
//=============================================================================
enum class SPIBaudRatePrescaler {
    DIV_2 = 0x00,   // PCLK / 2
    DIV_4 = 0x01,   // PCLK / 4
    DIV_8 = 0x02,   // PCLK / 8
    DIV_16 = 0x03,  // PCLK / 16
    DIV_32 = 0x04,  // PCLK / 32
    DIV_64 = 0x05,  // PCLK / 64
    DIV_128 = 0x06, // PCLK / 128
    DIV_256 = 0x07  // PCLK / 256
};

//=============================================================================
// SPI Clock Polarity and Phase
//=============================================================================
enum class SPIClockPolarity {
    LOW = 0,    // CK to 0 when idle
    HIGH = 1    // CK to 1 when idle
};

enum class SPIClockPhase {
    FIRST_EDGE = 0,     // Data captured on first clock edge
    SECOND_EDGE = 1     // Data captured on second clock edge
};

//=============================================================================
// SPI Data Size
//=============================================================================
enum class SPIDataSize {
    BITS_4 = 0x03,
    BITS_5 = 0x04,
    BITS_6 = 0x05,
    BITS_7 = 0x06,
    BITS_8 = 0x07,
    BITS_9 = 0x08,
    BITS_10 = 0x09,
    BITS_11 = 0x0A,
    BITS_12 = 0x0B,
    BITS_13 = 0x0C,
    BITS_14 = 0x0D,
    BITS_15 = 0x0E,
    BITS_16 = 0x0F
};

//=============================================================================
// SPI Frame Format
//=============================================================================
enum class SPIFrameFormat {
    MSB_FIRST = 0,
    LSB_FIRST = 1
};

//=============================================================================
// SPI Chip Select (NSS) Management
//=============================================================================
enum class SPINSSMode {
    SOFTWARE = 0,   // NSS managed by software
    HARDWARE = 1    // NSS managed by hardware
};

//=============================================================================
// SPI Configuration Structure
//=============================================================================
struct SPIConfig {
    SPIInstance instance;
    SPIMode mode;
    SPIBaudRatePrescaler baudRate;
    SPIClockPolarity clockPolarity;
    SPIClockPhase clockPhase;
    SPIDataSize dataSize;
    SPIFrameFormat frameFormat;
    SPINSSMode nssMode;
    bool enableCRC;
    bool enableDMA;
    bool enableInterrupt;
    
    SPIConfig() : instance(SPIInstance::SPI1),
                  mode(SPIMode::MASTER),
                  baudRate(SPIBaudRatePrescaler::DIV_8),
                  clockPolarity(SPIClockPolarity::LOW),
                  clockPhase(SPIClockPhase::FIRST_EDGE),
                  dataSize(SPIDataSize::BITS_8),
                  frameFormat(SPIFrameFormat::MSB_FIRST),
                  nssMode(SPINSSMode::SOFTWARE),
                  enableCRC(false),
                  enableDMA(false),
                  enableInterrupt(false) {}
};

//=============================================================================
// SPI Message Structure
//=============================================================================
struct SPIMessage {
    std::vector<uint8_t> txData;
    std::vector<uint8_t> rxData;
    uint32_t timeout;
    bool completed;
    bool error;
    
    SPIMessage() : timeout(1000), completed(false), error(false) {}
};

//=============================================================================
// SPI Peripheral Class
//=============================================================================
class SPIPeripheral {
private:
    SPIInstance instance;
    SPIConfig config;
    bool initialized;
    bool enabled;
    
    // Bus frequencies (from clock config)
    uint32_t pclk1;  // For SPI2, SPI3
    uint32_t pclk2;  // For SPI1, SPI4, SPI5, SPI6
    
    // TX/RX buffers
    std::queue<uint8_t> txBuffer;
    std::queue<uint8_t> rxBuffer;
    
    // DMA support
    bool dmaTxEnabled;
    bool dmaRxEnabled;
    uint8_t dmaTxStream;
    uint8_t dmaRxStream;
    
    // Callbacks
    std::function<void()> txCompleteCallback;
    std::function<void()> rxCompleteCallback;
    std::function<void(uint8_t)> dataReceivedCallback;
    
    // Statistics
    uint64_t bytesTransmitted;
    uint64_t bytesReceived;
    uint32_t errorCount;
    
    // Helper methods
    uint32_t getBaseAddress() const;
    std::string getInstanceName() const;
    uint32_t calculateBaudRate() const;
    
public:
    SPIPeripheral(SPIInstance inst);
    
    // Configuration
    bool init(const SPIConfig& cfg);
    void setClockFrequencies(uint32_t apb1, uint32_t apb2);
    void deinit();
    
    // Control
    bool enable();
    void disable();
    bool isEnabled() const { return enabled; }
    
    // Data transfer
    bool transmit(uint8_t data);
    bool transmit(const uint8_t* data, uint32_t length);
    bool transmit(const std::vector<uint8_t>& data);
    
    bool receive(uint8_t& data);
    uint32_t receive(uint8_t* buffer, uint32_t maxLength);
    std::vector<uint8_t> receive(uint32_t length);
    
    bool transmitReceive(uint8_t txData, uint8_t& rxData);
    bool transmitReceive(const uint8_t* txData, uint8_t* rxData, uint32_t length);
    
    // DMA transfers
    void enableDMA(uint8_t txStream, uint8_t rxStream);
    void disableDMA();
    bool startDMATransmit(const uint8_t* data, uint32_t length);
    bool startDMAReceive(uint8_t* buffer, uint32_t length);
    
    // Status
    bool isBusy() const;
    bool isTxEmpty() const;
    bool isRxNotEmpty() const;
    
    // Callback registration
    void onTransmitComplete(std::function<void()> callback);
    void onReceiveComplete(std::function<void()> callback);
    void onDataReceived(std::function<void(uint8_t)> callback);
    
    // Statistics
    uint64_t getBytesTransmitted() const { return bytesTransmitted; }
    uint64_t getBytesReceived() const { return bytesReceived; }
    uint32_t getErrorCount() const { return errorCount; }
    void resetStatistics();
    
    // Print configuration
    void printConfig() const;
    
    // Specific device communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t& value);
    bool writeReadRegister(uint8_t reg, uint8_t txValue, uint8_t& rxValue);
    
    // LCD specific functions
    void lcdWriteCommand(uint8_t cmd);
    void lcdWriteData(uint8_t data);
    void lcdWriteData(const uint8_t* data, uint32_t length);
    void lcdSetAddress(uint16_t x, uint16_t y);
    void lcdFillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
    
private:
    // Register manipulation
    void writeRegister(uint32_t reg, uint32_t value);
    uint32_t readRegister(uint32_t reg);
};

} // namespace STM32F429

#endif // STM32F429_SPI_CONFIG_H
