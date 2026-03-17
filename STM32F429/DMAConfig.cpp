/* --- DMAConfig.cpp --- */

/* ------------------------------------------
author: Pnbeldor
date: 3/12/2026
------------------------------------------ */

#include "DMAConfig.h"

#include <algorithm>

namespace STM32F429 {

//=============================================================================
// DMAPeripheral Implementation
//=============================================================================

DMAPeripheral::DMAPeripheral(DMAInstance inst)
    : instance(inst),
      initialized(false),
      enabled(false) {
    
    // Initialize all streams
    for (int i = 0; i <= 7; i++) {
        streams[static_cast<DMAStream>(i)] = StreamControl();
    }
}

uint32_t DMAPeripheral::getBaseAddress() const {
    switch(instance) {
        case DMAInstance::DMA1: return 0x40026000;
        case DMAInstance::DMA2: return 0x40026400;
        default: return 0;
    }
}

std::string DMAPeripheral::getInstanceName() const {
    switch(instance) {
        case DMAInstance::DMA1: return "DMA1";
        case DMAInstance::DMA2: return "DMA2";
        default: return "Unknown";
    }
}

uint32_t DMAPeripheral::getStreamOffset(DMAStream stream) const {
    return 0x10 + (static_cast<uint32_t>(stream) * 0x18);
}

bool DMAPeripheral::init() {
    initialized = true;
    std::cout << "Initialized " << getInstanceName() << std::endl;
    return true;
}

void DMAPeripheral::deinit() {
    enabled = false;
    initialized = false;
    std::cout << "Deinitialized " << getInstanceName() << std::endl;
}

bool DMAPeripheral::configureStream(DMAStream stream, const DMAStreamConfig& config) {
    auto it = streams.find(stream);
    if (it == streams.end()) {
        std::cerr << "Error: Invalid DMA stream" << std::endl;
        return false;
    }
    
    it->second.config = config;
    
    std::cout << getInstanceName() << " Stream " << static_cast<int>(stream) 
              << " configured" << std::endl;
    return true;
}

bool DMAPeripheral::setStreamPeripheralAddress(DMAStream stream, uint32_t address) {
    auto it = streams.find(stream);
    if (it == streams.end()) return false;
    
    it->second.periphAddress = address;
    return true;
}

bool DMAPeripheral::setStreamMemoryAddress(DMAStream stream, uint32_t address0, uint32_t address1) {
    auto it = streams.find(stream);
    if (it == streams.end()) return false;
    
    it->second.memoryAddress0 = address0;
    it->second.memoryAddress1 = address1;
    return true;
}

bool DMAPeripheral::setStreamDataLength(DMAStream stream, uint32_t length) {
    auto it = streams.find(stream);
    if (it == streams.end()) return false;
    
    it->second.dataLength = length;
    return true;
}

bool DMAPeripheral::enable() {
    enabled = true;
    std::cout << "Enabled " << getInstanceName() << std::endl;
    return true;
}

void DMAPeripheral::disable() {
    enabled = false;
    
    // Stop all streams
    for (int i = 0; i <= 7; i++) {
        stopStream(static_cast<DMAStream>(i));
    }
    
    std::cout << "Disabled " << getInstanceName() << std::endl;
}

bool DMAPeripheral::startStream(DMAStream stream) {
    auto it = streams.find(stream);
    if (it == streams.end()) {
        return false;
    }
    
    it->second.status.active = true;
    it->second.status.transferComplete = false;
    it->second.status.halfTransfer = false;
    it->second.status.remainingBytes = it->second.dataLength;
    it->second.status.totalBytes = it->second.dataLength;
    
    std::cout << getInstanceName() << " Stream " << static_cast<int>(stream) 
              << " started: " << it->second.dataLength << " bytes" << std::endl;
    
    return true;
}

bool DMAPeripheral::stopStream(DMAStream stream) {
    auto it = streams.find(stream);
    if (it == streams.end()) {
        return false;
    }
    
    it->second.status.active = false;
    return true;
}

bool DMAPeripheral::suspendStream(DMAStream stream) {
    auto it = streams.find(stream);
    if (it == streams.end()) {
        return false;
    }
    
    it->second.status.active = false;
    return true;
}

bool DMAPeripheral::resumeStream(DMAStream stream) {
    auto it = streams.find(stream);
    if (it == streams.end()) {
        return false;
    }
    
    it->second.status.active = true;
    return true;
}

DMAStatus DMAPeripheral::getStreamStatus(DMAStream stream) const {
    auto it = streams.find(stream);
    if (it != streams.end()) {
        return it->second.status;
    }
    return DMAStatus();
}

bool DMAPeripheral::isStreamActive(DMAStream stream) const {
    auto it = streams.find(stream);
    if (it != streams.end()) {
        return it->second.status.active;
    }
    return false;
}

bool DMAPeripheral::isTransferComplete(DMAStream stream) const {
    auto it = streams.find(stream);
    if (it != streams.end()) {
        return it->second.status.transferComplete;
    }
    return false;
}

void DMAPeripheral::onTransferComplete(DMAStream stream, std::function<void()> callback) {
    auto it = streams.find(stream);
    if (it != streams.end()) {
        it->second.completeCallback = callback;
    }
}

void DMAPeripheral::onHalfTransfer(DMAStream stream, std::function<void()> callback) {
    auto it = streams.find(stream);
    if (it != streams.end()) {
        it->second.halfCompleteCallback = callback;
    }
}

void DMAPeripheral::onError(DMAStream stream, std::function<void()> callback) {
    auto it = streams.find(stream);
    if (it != streams.end()) {
        it->second.errorCallback = callback;
    }
}

bool DMAPeripheral::memoryToMemoryTransfer(DMAStream stream, uint32_t src, uint32_t dst, uint32_t length) {
    std::cout << getInstanceName() << " Memory to memory transfer: " << length 
              << " bytes from 0x" << std::hex << src << " to 0x" << dst << std::dec << std::endl;
    
    // Simulate transfer
    auto it = streams.find(stream);
    if (it != streams.end()) {
        it->second.status.active = true;
        it->second.status.transferComplete = false;
        it->second.status.remainingBytes = length;
        it->second.status.totalBytes = length;
        
        // Simulate transfer completion
        it->second.status.transferComplete = true;
        it->second.status.active = false;
        
        if (it->second.completeCallback) {
            it->second.completeCallback();
        }
    }
    
    return true;
}

void DMAPeripheral::configureForSPI_TX(DMAStream stream, DMAChannel channel, DMAPriority priority) {
    DMAStreamConfig config;
    config.stream = stream;
    config.channel = channel;
    config.direction = DMADirection::MEMORY_TO_PERIPH;
    config.priority = priority;
    config.periphDataSize = DMADataSize::BYTE;
    config.memoryDataSize = DMADataSize::BYTE;
    config.periphInc = false;
    config.memoryInc = true;
    config.circular = false;
    config.fifoEnabled = true;
    
    configureStream(stream, config);
    std::cout << getInstanceName() << ": Configured for SPI TX on Stream " 
              << static_cast<int>(stream) << std::endl;
}

void DMAPeripheral::configureForSPI_RX(DMAStream stream, DMAChannel channel, DMAPriority priority) {
    DMAStreamConfig config;
    config.stream = stream;
    config.channel = channel;
    config.direction = DMADirection::PERIPH_TO_MEMORY;
    config.priority = priority;
    config.periphDataSize = DMADataSize::BYTE;
    config.memoryDataSize = DMADataSize::BYTE;
    config.periphInc = false;
    config.memoryInc = true;
    config.circular = false;
    config.fifoEnabled = true;
    
    configureStream(stream, config);
    std::cout << getInstanceName() << ": Configured for SPI RX on Stream " 
              << static_cast<int>(stream) << std::endl;
}

void DMAPeripheral::configureForI2C_TX(DMAStream stream, DMAChannel channel, DMAPriority priority) {
    DMAStreamConfig config;
    config.stream = stream;
    config.channel = channel;
    config.direction = DMADirection::MEMORY_TO_PERIPH;
    config.priority = priority;
    config.periphDataSize = DMADataSize::BYTE;
    config.memoryDataSize = DMADataSize::BYTE;
    config.periphInc = false;
    config.memoryInc = true;
    config.circular = false;
    config.fifoEnabled = true;
    
    configureStream(stream, config);
    std::cout << getInstanceName() << ": Configured for I2C TX on Stream " 
              << static_cast<int>(stream) << std::endl;
}

void DMAPeripheral::configureForI2C_RX(DMAStream stream, DMAChannel channel, DMAPriority priority) {
    DMAStreamConfig config;
    config.stream = stream;
    config.channel = channel;
    config.direction = DMADirection::PERIPH_TO_MEMORY;
    config.priority = priority;
    config.periphDataSize = DMADataSize::BYTE;
    config.memoryDataSize = DMADataSize::BYTE;
    config.periphInc = false;
    config.memoryInc = true;
    config.circular = false;
    config.fifoEnabled = true;
    
    configureStream(stream, config);
    std::cout << getInstanceName() << ": Configured for I2C RX on Stream " 
              << static_cast<int>(stream) << std::endl;
}

void DMAPeripheral::configureForUSART_TX(DMAStream stream, DMAChannel channel, DMAPriority priority) {
    DMAStreamConfig config;
    config.stream = stream;
    config.channel = channel;
    config.direction = DMADirection::MEMORY_TO_PERIPH;
    config.priority = priority;
    config.periphDataSize = DMADataSize::BYTE;
    config.memoryDataSize = DMADataSize::BYTE;
    config.periphInc = false;
    config.memoryInc = true;
    config.circular = false;
    config.fifoEnabled = true;
    
    configureStream(stream, config);
    std::cout << getInstanceName() << ": Configured for USART TX on Stream " 
              << static_cast<int>(stream) << std::endl;
}

void DMAPeripheral::configureForUSART_RX(DMAStream stream, DMAChannel channel, DMAPriority priority) {
    DMAStreamConfig config;
    config.stream = stream;
    config.channel = channel;
    config.direction = DMADirection::PERIPH_TO_MEMORY;
    config.priority = priority;
    config.periphDataSize = DMADataSize::BYTE;
    config.memoryDataSize = DMADataSize::BYTE;
    config.periphInc = false;
    config.memoryInc = true;
    config.circular = false;
    config.fifoEnabled = true;
    
    configureStream(stream, config);
    std::cout << getInstanceName() << ": Configured for USART RX on Stream " 
              << static_cast<int>(stream) << std::endl;
}

void DMAPeripheral::configureForADC(DMAStream stream, DMAChannel channel, DMAPriority priority) {
    DMAStreamConfig config;
    config.stream = stream;
    config.channel = channel;
    config.direction = DMADirection::PERIPH_TO_MEMORY;
    config.priority = priority;
    config.periphDataSize = DMADataSize::HALF_WORD;  // ADC is 12-bit
    config.memoryDataSize = DMADataSize::HALF_WORD;
    config.periphInc = false;
    config.memoryInc = true;
    config.circular = true;  // ADC often uses circular mode
    config.fifoEnabled = true;
    
    configureStream(stream, config);
    std::cout << getInstanceName() << ": Configured for ADC on Stream " 
              << static_cast<int>(stream) << std::endl;
}

void DMAPeripheral::configureForDAC(DMAStream stream, DMAChannel channel, DMAPriority priority) {
    DMAStreamConfig config;
    config.stream = stream;
    config.channel = channel;
    config.direction = DMADirection::MEMORY_TO_PERIPH;
    config.priority = priority;
    config.periphDataSize = DMADataSize::HALF_WORD;  // DAC is 12-bit
    config.memoryDataSize = DMADataSize::HALF_WORD;
    config.periphInc = false;
    config.memoryInc = true;
    config.circular = true;
    config.fifoEnabled = true;
    
    configureStream(stream, config);
    std::cout << getInstanceName() << ": Configured for DAC on Stream " 
              << static_cast<int>(stream) << std::endl;
}

void DMAPeripheral::configureForSDIO(DMAStream stream, DMAChannel channel, DMAPriority priority) {
    DMAStreamConfig config;
    config.stream = stream;
    config.channel = channel;
    config.direction = DMADirection::MEMORY_TO_PERIPH;  // Can be both directions
    config.priority = priority;
    config.periphDataSize = DMADataSize::WORD;
    config.memoryDataSize = DMADataSize::WORD;
    config.periphInc = false;
    config.memoryInc = true;
    config.fifoEnabled = true;
    config.periphBurst = DMABurstSize::INCR4;
    config.memoryBurst = DMABurstSize::INCR4;
    
    configureStream(stream, config);
    std::cout << getInstanceName() << ": Configured for SDIO on Stream " 
              << static_cast<int>(stream) << std::endl;
}

void DMAPeripheral::configureForDCMI(DMAStream stream, DMAChannel channel, DMAPriority priority) {
    DMAStreamConfig config;
    config.stream = stream;
    config.channel = channel;
    config.direction = DMADirection::PERIPH_TO_MEMORY;
    config.priority = priority;
    config.periphDataSize = DMADataSize::WORD;
    config.memoryDataSize = DMADataSize::WORD;
    config.periphInc = false;
    config.memoryInc = true;
    config.circular = true;
    config.fifoEnabled = true;
    config.periphBurst = DMABurstSize::INCR4;
    config.memoryBurst = DMABurstSize::INCR4;
    
    configureStream(stream, config);
    std::cout << getInstanceName() << ": Configured for DCMI on Stream " 
              << static_cast<int>(stream) << std::endl;
}

void DMAPeripheral::printConfig() const {
    std::cout << "\n=== " << getInstanceName() << " Configuration ===" << std::endl;
    std::cout << "Base Address: 0x" << std::hex << getBaseAddress() << std::dec << std::endl;
    std::cout << "Initialized: " << (initialized ? "Yes" : "No") << std::endl;
    std::cout << "Enabled: " << (enabled ? "Yes" : "No") << std::endl;
    
    for (int i = 0; i <= 7; i++) {
        DMAStream stream = static_cast<DMAStream>(i);
        auto it = streams.find(stream);
        if (it != streams.end() && it->second.status.active) {
            printStreamConfig(stream);
        }
    }
}

void DMAPeripheral::printStreamConfig(DMAStream stream) const {
    auto it = streams.find(stream);
    if (it == streams.end()) return;
    
    const auto& sc = it->second;
    std::cout << "\n  Stream " << static_cast<int>(stream) << ":" << std::endl;
    std::cout << "    Active: " << (sc.status.active ? "Yes" : "No") << std::endl;
    std::cout << "    Direction: " << static_cast<int>(sc.config.direction) << std::endl;
    std::cout << "    Data Length: " << sc.dataLength << " bytes" << std::endl;
    std::cout << "    Remaining: " << sc.status.remainingBytes << " bytes" << std::endl;
    std::cout << "    Complete: " << (sc.status.transferComplete ? "Yes" : "No") << std::endl;
}

} // namespace STM32F429
