#include "I2CConfig.h"

#include <algorithm>
#include <random>
#include <thread>

namespace STM32F429 {

//=============================================================================
// I2CPeripheral Implementation
//=============================================================================

I2CPeripheral::I2CPeripheral(I2CInstance inst)
    : instance(inst),
      initialized(false),
      enabled(false),
      pclk1(45000000),
      dmaTxEnabled(false),
      dmaRxEnabled(false),
      dmaTxStream(0),
      dmaRxStream(0),
      bytesTransmitted(0),
      bytesReceived(0),
      errorCount(0),
      messageCount(0) {
    
    config.instance = inst;
}

uint32_t I2CPeripheral::getBaseAddress() const {
    switch(instance) {
        case I2CInstance::I2C1: return 0x40005400;
        case I2CInstance::I2C2: return 0x40005800;
        case I2CInstance::I2C3: return 0x40005C00;
        default: return 0;
    }
}

std::string I2CPeripheral::getInstanceName() const {
    switch(instance) {
        case I2CInstance::I2C1: return "I2C1";
        case I2CInstance::I2C2: return "I2C2";
        case I2CInstance::I2C3: return "I2C3";
        default: return "Unknown";
    }
}

uint32_t I2CPeripheral::calculateClockSpeed() const {
    if (config.mode == I2CMode::STANDARD) {
        return 100000;  // 100 kHz
    } else {
        return 400000;  // 400 kHz
    }
}

uint32_t I2CPeripheral::calculateTiming() const {
    // Simplified timing calculation
    // In real implementation, this would calculate CCR and TRISE values
    uint32_t clockSpeed = calculateClockSpeed();
    uint32_t timing = pclk1 / clockSpeed;
    
    if (config.mode == I2CMode::FAST) {
        timing /= (config.dutyCycle == I2CDutyCycle::DUTY_2 ? 2 : 16);
    }
    
    return timing;
}

bool I2CPeripheral::init(const I2CConfig& cfg) {
    config = cfg;
    initialized = true;
    
    std::cout << "Initializing " << getInstanceName() << std::endl;
    std::cout << "  Mode: " << (config.mode == I2CMode::STANDARD ? "STANDARD (100 kHz)" : "FAST (400 kHz)") << std::endl;
    std::cout << "  Own Address: 0x" << std::hex << config.ownAddress << std::dec << std::endl;
    std::cout << "  Addressing: " << (config.addressingMode == I2CAddressingMode::_7BIT ? "7-bit" : "10-bit") << std::endl;
    
    return true;
}

void I2CPeripheral::setClockFrequency(uint32_t apb1) {
    pclk1 = apb1;
}

void I2CPeripheral::deinit() {
    disable();
    initialized = false;
    std::cout << "Deinitialized " << getInstanceName() << std::endl;
}

bool I2CPeripheral::enable() {
    if (!initialized) {
        std::cerr << "Error: " << getInstanceName() << " not initialized" << std::endl;
        return false;
    }
    
    enabled = true;
    std::cout << "Enabled " << getInstanceName() << std::endl;
    return true;
}

void I2CPeripheral::disable() {
    enabled = false;
    std::cout << "Disabled " << getInstanceName() << std::endl;
}

bool I2CPeripheral::masterTransmit(uint16_t address, const uint8_t* data, uint32_t length, bool stop) {
    if (!enabled) {
        errorCount++;
        return false;
    }
    
    std::cout << getInstanceName() << " Master Transmit to 0x" << std::hex << address 
              << " (" << std::dec << length << " bytes)" << std::endl;
    
    for (uint32_t i = 0; i < length; i++) {
        std::cout << "  Byte " << i << ": 0x" << std::hex << (int)data[i] << std::dec;
        if (data[i] >= 32 && data[i] <= 126) {
            std::cout << " '" << (char)data[i] << "'";
        }
        std::cout << std::endl;
    }
    
    bytesTransmitted += length;
    messageCount++;
    
    // Simulate device response for known slaves
    auto it = slaveDevices.find(address);
    if (it != slaveDevices.end()) {
        // Device acknowledged
        std::cout << "  Device at 0x" << std::hex << address << " ACK" << std::dec << std::endl;
    }
    
    return true;
}

bool I2CPeripheral::masterTransmit(uint16_t address, const std::vector<uint8_t>& data, bool stop) {
    return masterTransmit(address, data.data(), data.size(), stop);
}

bool I2CPeripheral::masterReceive(uint16_t address, uint8_t* buffer, uint32_t length, bool stop) {
    if (!enabled) {
        errorCount++;
        return false;
    }
    
    std::cout << getInstanceName() << " Master Receive from 0x" << std::hex << address 
              << " (" << std::dec << length << " bytes)" << std::endl;
    
    auto it = slaveDevices.find(address);
    if (it != slaveDevices.end()) {
        // Provide data from simulated device
        uint32_t available = std::min(length, (uint32_t)it->second.size());
        for (uint32_t i = 0; i < available; i++) {
            buffer[i] = it->second[i];
        }
        bytesReceived += available;
        
        // Show received data
        for (uint32_t i = 0; i < available; i++) {
            std::cout << "  Byte " << i << ": 0x" << std::hex << (int)buffer[i] << std::dec;
            if (buffer[i] >= 32 && buffer[i] <= 126) {
                std::cout << " '" << (char)buffer[i] << "'";
            }
            std::cout << std::endl;
        }
    } else {
        // No slave found, return dummy data
        for (uint32_t i = 0; i < length; i++) {
            buffer[i] = 0xFF;
        }
        std::cout << "  Warning: No device at address 0x" << std::hex << address << std::dec << std::endl;
    }
    
    messageCount++;
    return true;
}

std::vector<uint8_t> I2CPeripheral::masterReceive(uint16_t address, uint32_t length, bool stop) {
    std::vector<uint8_t> buffer(length);
    if (masterReceive(address, buffer.data(), length, stop)) {
        return buffer;
    }
    return std::vector<uint8_t>();
}

bool I2CPeripheral::masterWriteRegister(uint16_t address, uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    return masterTransmit(address, data, 2);
}

bool I2CPeripheral::masterReadRegister(uint16_t address, uint8_t reg, uint8_t& value) {
    if (!masterTransmit(address, &reg, 1)) {
        return false;
    }
    
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    
    uint8_t buffer[1];
    if (!masterReceive(address, buffer, 1)) {
        return false;
    }
    
    value = buffer[0];
    return true;
}

bool I2CPeripheral::masterReadRegisters(uint16_t address, uint8_t reg, uint8_t* buffer, uint32_t length) {
    if (!masterTransmit(address, &reg, 1)) {
        return false;
    }
    
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    
    return masterReceive(address, buffer, length);
}

void I2CPeripheral::enableDMA(uint8_t txStream, uint8_t rxStream) {
    dmaTxEnabled = true;
    dmaRxEnabled = true;
    dmaTxStream = txStream;
    dmaRxStream = rxStream;
    config.enableDMA = true;
    std::cout << getInstanceName() << " DMA enabled (TX Stream " 
              << (int)txStream << ", RX Stream " << (int)rxStream << ")" << std::endl;
}

void I2CPeripheral::disableDMA() {
    dmaTxEnabled = false;
    dmaRxEnabled = false;
    config.enableDMA = false;
    std::cout << getInstanceName() << " DMA disabled" << std::endl;
}

bool I2CPeripheral::startDMATransmit(uint16_t address, const uint8_t* data, uint32_t length) {
    if (!dmaTxEnabled) {
        std::cerr << "Error: DMA TX not enabled for " << getInstanceName() << std::endl;
        return false;
    }
    
    std::cout << getInstanceName() << " DMA transmit: " << length << " bytes to 0x" 
              << std::hex << address << std::dec << std::endl;
    return masterTransmit(address, data, length);
}

bool I2CPeripheral::startDMAReceive(uint16_t address, uint8_t* buffer, uint32_t length) {
    if (!dmaRxEnabled) {
        std::cerr << "Error: DMA RX not enabled for " << getInstanceName() << std::endl;
        return false;
    }
    
    std::cout << getInstanceName() << " DMA receive: " << length << " bytes from 0x" 
              << std::hex << address << std::dec << std::endl;
    return masterReceive(address, buffer, length);
}

bool I2CPeripheral::isBusy() const {
    return false;  // Simplified
}

bool I2CPeripheral::isTxEmpty() const {
    return true;  // Simplified
}

bool I2CPeripheral::isRxNotEmpty() const {
    return false;  // Simplified
}

void I2CPeripheral::onMessageComplete(std::function<void(I2CMessage&)> callback) {
    messageCompleteCallback = callback;
}

void I2CPeripheral::onDataReceived(std::function<void(uint16_t, uint8_t)> callback) {
    dataReceivedCallback = callback;
}

void I2CPeripheral::resetStatistics() {
    bytesTransmitted = 0;
    bytesReceived = 0;
    errorCount = 0;
    messageCount = 0;
}

void I2CPeripheral::addSlaveDevice(uint16_t address, const std::vector<uint8_t>& data) {
    slaveDevices[address] = data;
    std::cout << getInstanceName() << ": Added simulated device at 0x" 
              << std::hex << address << std::dec << std::endl;
}

void I2CPeripheral::simulateTemperatureSensor(uint16_t address) {
    // Simulate a temperature sensor (e.g., LM75)
    std::vector<uint8_t> tempData(2);
    
    // Generate random temperature between 20-30°C
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> temp(2000, 3000);
    
    int16_t temperature = temp(gen);
    tempData[0] = (temperature >> 8) & 0xFF;
    tempData[1] = temperature & 0xFF;
    
    addSlaveDevice(address, tempData);
    std::cout << "  Temperature sensor at 0x" << std::hex << address 
              << " = " << std::dec << (temperature / 100.0) << "°C" << std::endl;
}

void I2CPeripheral::simulateEEPROM(uint16_t address, uint32_t size) {
    // Simulate an EEPROM (e.g., 24LCxx)
    std::vector<uint8_t> eepromData(size);
    for (uint32_t i = 0; i < size; i++) {
        eepromData[i] = i & 0xFF;
    }
    
    addSlaveDevice(address, eepromData);
    std::cout << "  EEPROM at 0x" << std::hex << address 
              << " (" << std::dec << size << " bytes)" << std::endl;
}

void I2CPeripheral::simulateAccelerometer(uint16_t address) {
    // Simulate an accelerometer (e.g., LIS3DSH)
    std::vector<uint8_t> accelData(6);  // X, Y, Z each 16-bit
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> accel(-1000, 1000);
    
    int16_t x = accel(gen);
    int16_t y = accel(gen);
    int16_t z = accel(gen) + 1000;  // Gravity
    
    accelData[0] = x >> 8;
    accelData[1] = x & 0xFF;
    accelData[2] = y >> 8;
    accelData[3] = y & 0xFF;
    accelData[4] = z >> 8;
    accelData[5] = z & 0xFF;
    
    addSlaveDevice(address, accelData);
    std::cout << "  Accelerometer at 0x" << std::hex << address << std::dec << std::endl;
}

bool I2CPeripheral::readTemperature(uint16_t address, float& temperature) {
    uint8_t data[2];
    if (!masterReceive(address, data, 2)) {
        return false;
    }
    
    int16_t raw = (data[0] << 8) | data[1];
    temperature = raw / 100.0f;
    return true;
}

bool I2CPeripheral::readAccelerometer(uint16_t address, int16_t& x, int16_t& y, int16_t& z) {
    uint8_t data[6];
    if (!masterReceive(address, data, 6)) {
        return false;
    }
    
    x = (data[0] << 8) | data[1];
    y = (data[2] << 8) | data[3];
    z = (data[4] << 8) | data[5];
    return true;
}

bool I2CPeripheral::writeEEPROM(uint16_t address, uint16_t memAddress, uint8_t data) {
    uint8_t buffer[3] = {(uint8_t)(memAddress >> 8), (uint8_t)(memAddress & 0xFF), data};
    return masterTransmit(address, buffer, 3);
}

bool I2CPeripheral::readEEPROM(uint16_t address, uint16_t memAddress, uint8_t& data) {
    uint8_t addrBuffer[2] = {(uint8_t)(memAddress >> 8), (uint8_t)(memAddress & 0xFF)};
    
    if (!masterTransmit(address, addrBuffer, 2)) {
        return false;
    }
    
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    
    uint8_t buffer[1];
    if (!masterReceive(address, buffer, 1)) {
        return false;
    }
    
    data = buffer[0];
    return true;
}

void I2CPeripheral::handleInterrupt() {
    // Handle I2C interrupts
}

void I2CPeripheral::printConfig() const {
    std::cout << "\n=== " << getInstanceName() << " Configuration ===" << std::endl;
    std::cout << "Base Address: 0x" << std::hex << getBaseAddress() << std::dec << std::endl;
    std::cout << "Mode: " << (config.mode == I2CMode::STANDARD ? "STANDARD (100 kHz)" : "FAST (400 kHz)") << std::endl;
    std::cout << "Own Address: 0x" << std::hex << config.ownAddress << std::dec << std::endl;
    std::cout << "Addressing: " << (config.addressingMode == I2CAddressingMode::_7BIT ? "7-bit" : "10-bit") << std::endl;
    std::cout << "ACK: " << (config.ack == I2CAck::ENABLE ? "Enabled" : "Disabled") << std::endl;
    std::cout << "General Call: " << (config.enableGeneralCall ? "Enabled" : "Disabled") << std::endl;
    std::cout << "DMA: " << (config.enableDMA ? "Enabled" : "Disabled") << std::endl;
    std::cout << "Statistics: Messages=" << messageCount << ", TX=" << bytesTransmitted 
              << ", RX=" << bytesReceived << ", Errors=" << errorCount << std::endl;
}

} // namespace STM32F429
