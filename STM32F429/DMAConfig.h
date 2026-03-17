/* --- DMAConfig.h --- */

/* ------------------------------------------
Author: Pnbeldor
Date: 3/12/2026
------------------------------------------ */

#ifndef __DMA_CONFIG_H__
#define __DMA_CONFIG_H__

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <map>

namespace STM32F429 {

//=============================================================================
// DMA Instance Selection
//=============================================================================
enum class DMAInstance {
    DMA1,
    DMA2
};

//=============================================================================
// DMA Stream Selection (0-7)
//=============================================================================
enum class DMAStream {
    STREAM0 = 0,
    STREAM1 = 1,
    STREAM2 = 2,
    STREAM3 = 3,
    STREAM4 = 4,
    STREAM5 = 5,
    STREAM6 = 6,
    STREAM7 = 7
};

//=============================================================================
// DMA Channel Selection (0-7)
//=============================================================================
enum class DMAChannel {
    CHANNEL0 = 0,
    CHANNEL1 = 1,
    CHANNEL2 = 2,
    CHANNEL3 = 3,
    CHANNEL4 = 4,
    CHANNEL5 = 5,
    CHANNEL6 = 6,
    CHANNEL7 = 7
};

//=============================================================================
// DMA Direction
//=============================================================================
enum class DMADirection {
    PERIPH_TO_MEMORY = 0x00,
    MEMORY_TO_PERIPH = 0x01,
    MEMORY_TO_MEMORY = 0x02
};

//=============================================================================
// DMA Priority
//=============================================================================
enum class DMAPriority {
    LOW = 0x00,
    MEDIUM = 0x01,
    HIGH = 0x02,
    VERY_HIGH = 0x03
};

//=============================================================================
// DMA Data Size
//=============================================================================
enum class DMADataSize {
    BYTE = 0x00,
    HALF_WORD = 0x01,
    WORD = 0x02
};

//=============================================================================
// DMA Burst Size
//=============================================================================
enum class DMABurstSize {
    SINGLE = 0x00,
    INCR4 = 0x01,
    INCR8 = 0x02,
    INCR16 = 0x03
};

//=============================================================================
// DMA FIFO Threshold
//=============================================================================
enum class DMAFIFOThreshold {
    QUARTER = 0x00,
    HALF = 0x01,
    THREE_QUARTERS = 0x02,
    FULL = 0x03
};

//=============================================================================
// DMA Stream Configuration Structure
//=============================================================================
struct DMAStreamConfig {
    DMAStream stream;
    DMAChannel channel;
    DMADirection direction;
    DMAPriority priority;
    DMADataSize periphDataSize;
    DMADataSize memoryDataSize;
    bool periphInc;           // Increment peripheral address
    bool memoryInc;           // Increment memory address
    bool circular;            // Circular mode
    bool doubleBuffer;        // Double buffer mode
    bool periphFlowControl;   // Peripheral as flow controller
    DMABurstSize periphBurst; // Peripheral burst size
    DMABurstSize memoryBurst; // Memory burst size
    DMAFIFOThreshold fifoThreshold;
    bool fifoEnabled;
    uint32_t fifoErrorInterrupt;
    
    DMAStreamConfig() : stream(DMAStream::STREAM0),
                        channel(DMAChannel::CHANNEL0),
                        direction(DMADirection::MEMORY_TO_PERIPH),
                        priority(DMAPriority::MEDIUM),
                        periphDataSize(DMADataSize::BYTE),
                        memoryDataSize(DMADataSize::BYTE),
                        periphInc(false),
                        memoryInc(true),
                        circular(false),
                        doubleBuffer(false),
                        periphFlowControl(false),
                        periphBurst(DMABurstSize::SINGLE),
                        memoryBurst(DMABurstSize::SINGLE),
                        fifoThreshold(DMAFIFOThreshold::HALF),
                        fifoEnabled(true),
                        fifoErrorInterrupt(true) {}
};

//=============================================================================
// DMA Transfer Status
//=============================================================================
struct DMAStatus {
    bool active;
    bool transferComplete;
    bool halfTransfer;
    bool fifoError;
    bool directModeError;
    uint32_t remainingBytes;
    uint32_t totalBytes;
    
    DMAStatus() : active(false), transferComplete(false), halfTransfer(false),
                  fifoError(false), directModeError(false),
                  remainingBytes(0), totalBytes(0) {}
};

//=============================================================================
// Peripheral DMA Mapping
//=============================================================================
struct DMAPeripheralMapping {
    std::string peripheral;
    DMAInstance instance;
    DMAStream stream;
    DMAChannel channel;
};

//=============================================================================
// DMA Peripheral Class
//=============================================================================
class DMAPeripheral {
private:
    DMAInstance instance;
    bool initialized;
    bool enabled;
    
    struct StreamControl {
        DMAStreamConfig config;
        DMAStatus status;
        uint32_t periphAddress;
        uint32_t memoryAddress0;
        uint32_t memoryAddress1;
        uint32_t dataLength;
        uint32_t currentMemoryBuffer;
        
        // Callbacks
        std::function<void()> completeCallback;
        std::function<void()> halfCompleteCallback;
        std::function<void()> errorCallback;
        
        StreamControl() : periphAddress(0), memoryAddress0(0), 
                          memoryAddress1(0), dataLength(0), currentMemoryBuffer(0) {}
    };
    
    std::map<DMAStream, StreamControl> streams;
    
    // Helper methods
    uint32_t getBaseAddress() const;
    std::string getInstanceName() const;
    uint32_t getStreamOffset(DMAStream stream) const;
    
public:
    DMAPeripheral(DMAInstance inst);
    
    // Configuration
    bool init();
    void deinit();
    
    // Stream configuration
    bool configureStream(DMAStream stream, const DMAStreamConfig& config);
    bool setStreamPeripheralAddress(DMAStream stream, uint32_t address);
    bool setStreamMemoryAddress(DMAStream stream, uint32_t address0, uint32_t address1 = 0);
    bool setStreamDataLength(DMAStream stream, uint32_t length);
    
    // Control
    bool enable();
    void disable();
    bool startStream(DMAStream stream);
    bool stopStream(DMAStream stream);
    bool suspendStream(DMAStream stream);
    bool resumeStream(DMAStream stream);
    
    // Status
    DMAStatus getStreamStatus(DMAStream stream) const;
    bool isStreamActive(DMAStream stream) const;
    bool isTransferComplete(DMAStream stream) const;
    
    // Callbacks
    void onTransferComplete(DMAStream stream, std::function<void()> callback);
    void onHalfTransfer(DMAStream stream, std::function<void()> callback);
    void onError(DMAStream stream, std::function<void()> callback);
    
    // Memory operations
    bool memoryToMemoryTransfer(DMAStream stream, uint32_t src, uint32_t dst, uint32_t length);
    
    // Peripheral specific configurations
    void configureForSPI_TX(DMAStream stream, DMAChannel channel, DMAPriority priority = DMAPriority::MEDIUM);
    void configureForSPI_RX(DMAStream stream, DMAChannel channel, DMAPriority priority = DMAPriority::MEDIUM);
    void configureForI2C_TX(DMAStream stream, DMAChannel channel, DMAPriority priority = DMAPriority::MEDIUM);
    void configureForI2C_RX(DMAStream stream, DMAChannel channel, DMAPriority priority = DMAPriority::MEDIUM);
    void configureForUSART_TX(DMAStream stream, DMAChannel channel, DMAPriority priority = DMAPriority::MEDIUM);
    void configureForUSART_RX(DMAStream stream, DMAChannel channel, DMAPriority priority = DMAPriority::MEDIUM);
    void configureForADC(DMAStream stream, DMAChannel channel, DMAPriority priority = DMAPriority::MEDIUM);
    void configureForDAC(DMAStream stream, DMAChannel channel, DMAPriority priority = DMAPriority::MEDIUM);
    void configureForSDIO(DMAStream stream, DMAChannel channel, DMAPriority priority = DMAPriority::MEDIUM);
    void configureForDCMI(DMAStream stream, DMAChannel channel, DMAPriority priority = DMAPriority::MEDIUM);
    
    // Print configuration
    void printConfig() const;
    void printStreamConfig(DMAStream stream) const;
    
    // Static mapping tables
    static DMAPeripheralMapping getSPI1_TX() { return {"SPI1_TX", DMAInstance::DMA2, DMAStream::STREAM3, DMAChannel::CHANNEL3}; }
    static DMAPeripheralMapping getSPI1_RX() { return {"SPI1_RX", DMAInstance::DMA2, DMAStream::STREAM2, DMAChannel::CHANNEL3}; }
    static DMAPeripheralMapping getSPI2_TX() { return {"SPI2_TX", DMAInstance::DMA1, DMAStream::STREAM4, DMAChannel::CHANNEL0}; }
    static DMAPeripheralMapping getSPI2_RX() { return {"SPI2_RX", DMAInstance::DMA1, DMAStream::STREAM3, DMAChannel::CHANNEL0}; }
    static DMAPeripheralMapping getSPI3_TX() { return {"SPI3_TX", DMAInstance::DMA1, DMAStream::STREAM5, DMAChannel::CHANNEL0}; }
    static DMAPeripheralMapping getSPI3_RX() { return {"SPI3_RX", DMAInstance::DMA1, DMAStream::STREAM2, DMAChannel::CHANNEL0}; }
    
    static DMAPeripheralMapping getI2C1_TX() { return {"I2C1_TX", DMAInstance::DMA1, DMAStream::STREAM6, DMAChannel::CHANNEL1}; }
    static DMAPeripheralMapping getI2C1_RX() { return {"I2C1_RX", DMAInstance::DMA1, DMAStream::STREAM5, DMAChannel::CHANNEL1}; }
    static DMAPeripheralMapping getI2C2_TX() { return {"I2C2_TX", DMAInstance::DMA1, DMAStream::STREAM7, DMAChannel::CHANNEL7}; }
    static DMAPeripheralMapping getI2C2_RX() { return {"I2C2_RX", DMAInstance::DMA1, DMAStream::STREAM2, DMAChannel::CHANNEL7}; }
    
    static DMAPeripheralMapping getUSART1_TX() { return {"USART1_TX", DMAInstance::DMA2, DMAStream::STREAM7, DMAChannel::CHANNEL4}; }
    static DMAPeripheralMapping getUSART1_RX() { return {"USART1_RX", DMAInstance::DMA2, DMAStream::STREAM5, DMAChannel::CHANNEL4}; }
    static DMAPeripheralMapping getUSART2_TX() { return {"USART2_TX", DMAInstance::DMA1, DMAStream::STREAM6, DMAChannel::CHANNEL4}; }
    static DMAPeripheralMapping getUSART2_RX() { return {"USART2_RX", DMAInstance::DMA1, DMAStream::STREAM5, DMAChannel::CHANNEL4}; }
    
    static DMAPeripheralMapping getADC1() { return {"ADC1", DMAInstance::DMA2, DMAStream::STREAM0, DMAChannel::CHANNEL0}; }
    static DMAPeripheralMapping getADC2() { return {"ADC2", DMAInstance::DMA2, DMAStream::STREAM2, DMAChannel::CHANNEL1}; }
    static DMAPeripheralMapping getADC3() { return {"ADC3", DMAInstance::DMA2, DMAStream::STREAM1, DMAChannel::CHANNEL2}; }
    
    static DMAPeripheralMapping getDAC1() { return {"DAC1", DMAInstance::DMA1, DMAStream::STREAM5, DMAChannel::CHANNEL7}; }
    static DMAPeripheralMapping getDAC2() { return {"DAC2", DMAInstance::DMA1, DMAStream::STREAM6, DMAChannel::CHANNEL7}; }
    
    static DMAPeripheralMapping getSDIO() { return {"SDIO", DMAInstance::DMA2, DMAStream::STREAM4, DMAChannel::CHANNEL4}; }
    static DMAPeripheralMapping getDCMI() { return {"DCMI", DMAInstance::DMA2, DMAStream::STREAM1, DMAChannel::CHANNEL1}; }
};

} // namespace STM32F429

#endif // __DMA_CONFIG_H__
