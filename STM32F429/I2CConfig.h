#ifndef __I2C_CONFIG_H__
#define __I2C_CONFIG_H__

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <map>

namespace STM32F429 {

//=============================================================================
// I2C Instance Selection
//=============================================================================
enum class I2CInstance {
    I2C1,
    I2C2,
    I2C3
};

//=============================================================================
// I2C Mode
//=============================================================================
enum class I2CMode {
    STANDARD = 0,   // Up to 100 kHz
    FAST = 1        // Up to 400 kHz
};

//=============================================================================
// I2C Duty Cycle (for fast mode)
//=============================================================================
enum class I2CDutyCycle {
    DUTY_2 = 0,     // T_low / T_high = 2
    DUTY_16_9 = 1   // T_low / T_high = 16/9
};

//=============================================================================
// I2C Addressing Mode
//=============================================================================
enum class I2CAddressingMode {
    _7BIT = 0,
    _10BIT = 1
};

//=============================================================================
// I2C Acknowledgment
//=============================================================================
enum class I2CAck {
    DISABLE = 0,
    ENABLE = 1
};

//=============================================================================
// I2C Configuration Structure
//=============================================================================
struct I2CConfig {
    I2CInstance instance;
    I2CMode mode;
    I2CDutyCycle dutyCycle;
    I2CAddressingMode addressingMode;
    I2CAck ack;
    uint16_t ownAddress;
    bool enableGeneralCall;
    bool enableDMA;
    bool enableInterrupt;
    
    I2CConfig() : instance(I2CInstance::I2C1),
                  mode(I2CMode::STANDARD),
                  dutyCycle(I2CDutyCycle::DUTY_2),
                  addressingMode(I2CAddressingMode::_7BIT),
                  ack(I2CAck::ENABLE),
                  ownAddress(0x00),
                  enableGeneralCall(false),
                  enableDMA(false),
                  enableInterrupt(false) {}
};

//=============================================================================
// I2C Message Structure
//=============================================================================
struct I2CMessage {
    uint16_t address;
    std::vector<uint8_t> data;
    bool read;          // true for read, false for write
    bool completed;
    bool error;
    uint32_t timestamp;
    
    I2CMessage() : address(0), read(false), completed(false), error(false), timestamp(0) {}
};

//=============================================================================
// I2C Peripheral Class
//=============================================================================
class I2CPeripheral {
private:
    I2CInstance instance;
    I2CConfig config;
    bool initialized;
    bool enabled;
    
    // Bus frequencies
    uint32_t pclk1;  // APB1 clock for I2C1, I2C2, I2C3
    
    // DMA support
    bool dmaTxEnabled;
    bool dmaRxEnabled;
    uint8_t dmaTxStream;
    uint8_t dmaRxStream;
    
    // Callbacks
    std::function<void(I2CMessage&)> messageCompleteCallback;
    std::function<void(uint16_t, uint8_t)> dataReceivedCallback;
    
    // Statistics
    uint64_t bytesTransmitted;
    uint64_t bytesReceived;
    uint32_t errorCount;
    uint32_t messageCount;
    
    // Simulated EEPROM/Sensor data
    std::map<uint16_t, std::vector<uint8_t>> slaveDevices;
    
    // Helper methods
    uint32_t getBaseAddress() const;
    std::string getInstanceName() const;
    uint32_t calculateClockSpeed() const;
    
public:
    I2CPeripheral(I2CInstance inst);
    
    // Configuration
    bool init(const I2CConfig& cfg);
    void setClockFrequency(uint32_t apb1);
    void deinit();
    
    // Control
    bool enable();
    void disable();
    bool isEnabled() const { return enabled; }
    
    // Master operations
    bool masterTransmit(uint16_t address, const uint8_t* data, uint32_t length, bool stop = true);
    bool masterTransmit(uint16_t address, const std::vector<uint8_t>& data, bool stop = true);
    bool masterReceive(uint16_t address, uint8_t* buffer, uint32_t length, bool stop = true);
    std::vector<uint8_t> masterReceive(uint16_t address, uint32_t length, bool stop = true);
    
    bool masterWriteRegister(uint16_t address, uint8_t reg, uint8_t value);
    bool masterReadRegister(uint16_t address, uint8_t reg, uint8_t& value);
    bool masterReadRegisters(uint16_t address, uint8_t reg, uint8_t* buffer, uint32_t length);
    
    // DMA transfers
    void enableDMA(uint8_t txStream, uint8_t rxStream);
    void disableDMA();
    bool startDMATransmit(uint16_t address, const uint8_t* data, uint32_t length);
    bool startDMAReceive(uint16_t address, uint8_t* buffer, uint32_t length);
    
    // Status
    bool isBusy() const;
    bool isTxEmpty() const;
    bool isRxNotEmpty() const;
    
    // Callback registration
    void onMessageComplete(std::function<void(I2CMessage&)> callback);
    void onDataReceived(std::function<void(uint16_t, uint8_t)> callback);
    
    // Statistics
    uint64_t getBytesTransmitted() const { return bytesTransmitted; }
    uint64_t getBytesReceived() const { return bytesReceived; }
    uint32_t getErrorCount() const { return errorCount; }
    uint32_t getMessageCount() const { return messageCount; }
    void resetStatistics();
    
    // Simulated devices
    void addSlaveDevice(uint16_t address, const std::vector<uint8_t>& data);
    void simulateTemperatureSensor(uint16_t address);
    void simulateEEPROM(uint16_t address, uint32_t size);
    void simulateAccelerometer(uint16_t address);
    
    // Print configuration
    void printConfig() const;
    
    // Common device drivers
    bool readTemperature(uint16_t address, float& temperature);
    bool readAccelerometer(uint16_t address, int16_t& x, int16_t& y, int16_t& z);
    bool writeEEPROM(uint16_t address, uint16_t memAddress, uint8_t data);
    bool readEEPROM(uint16_t address, uint16_t memAddress, uint8_t& data);
    
private:
    void handleInterrupt();
    uint32_t calculateTiming() const;
};

} // namespace STM32F429


#endif  //__I2C_CONFIG_H__