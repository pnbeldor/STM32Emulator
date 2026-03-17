#ifndef __UART_CONFIG_H__
#define __UART_CONFIG_H__


#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <queue>
#include <map>
#include <thread>
#include <mutex>

namespace STM32F429 {

//=============================================================================
// UART/USART Instance Selection
//=============================================================================
enum class UARTInstance {
    USART1,     // APB2 max 90 MHz
    USART2,     // APB1 max 45 MHz
    USART3,     // APB1 max 45 MHz
    UART4,      // APB1 max 45 MHz
    UART5,      // APB1 max 45 MHz
    USART6,     // APB2 max 90 MHz
    UART7,      // APB1 max 45 MHz
    UART8       // APB1 max 45 MHz
};

//=============================================================================
// UART Mode
//=============================================================================
enum class UARTMode {
    TX,             // Transmit only
    RX,             // Receive only
    TX_RX,          // Transmit and receive
    HALF_DUPLEX,    // Half-duplex (single wire)
    LIN,            // LIN mode
    IRDA,           // IrDA mode
    SMART_CARD      // Smart card mode
};

//=============================================================================
// UART Baud Rate
//=============================================================================
enum class UARTBaudRate {
    BAUD_1200 = 1200,
    BAUD_2400 = 2400,
    BAUD_4800 = 4800,
    BAUD_9600 = 9600,
    BAUD_19200 = 19200,
    BAUD_38400 = 38400,
    BAUD_57600 = 57600,
    BAUD_115200 = 115200,
    BAUD_230400 = 230400,
    BAUD_460800 = 460800,
    BAUD_921600 = 921600,
    BAUD_2M = 2000000,
    BAUD_3M = 3000000,
    BAUD_4M = 4000000,
    BAUD_5M = 5000000,
    BAUD_6M = 6000000,
    BAUD_7M = 7000000,
    BAUD_8M = 8000000,
    BAUD_9M = 9000000,
    BAUD_10M = 10000000,
    BAUD_11M25 = 11250000   // Max for USART1/USART6
};

//=============================================================================
// UART Data Bits
//=============================================================================
enum class UARTDataBits {
    BITS_7 = 7,
    BITS_8 = 8,
    BITS_9 = 9
};

//=============================================================================
// UART Stop Bits
//=============================================================================
enum class UARTStopBits {
    STOP_0_5 = 0,
    STOP_1 = 1,
    STOP_1_5 = 2,
    STOP_2 = 3
};

//=============================================================================
// UART Parity
//=============================================================================
enum class UARTParity {
    NONE,
    EVEN,
    ODD
};

//=============================================================================
// UART Flow Control
//=============================================================================
enum class UARTFlowControl {
    NONE,
    RTS,
    CTS,
    RTS_CTS
};

//=============================================================================
// UART Oversampling
//=============================================================================
enum class UARTOversampling {
    OVERSAMPLING_16 = 16,   // Standard mode
    OVERSAMPLING_8 = 8      // High-speed mode
};

//=============================================================================
// UART Configuration Structure
//=============================================================================
struct UARTConfig {
    UARTInstance instance;
    UARTMode mode;
    UARTBaudRate baudRate;
    UARTDataBits dataBits;
    UARTStopBits stopBits;
    UARTParity parity;
    UARTFlowControl flowControl;
    UARTOversampling oversampling;
    bool enableDMA;
    bool enableInterrupt;
    uint32_t rxTimeout;         // Receive timeout in ms
    uint32_t txTimeout;         // Transmit timeout in ms
    uint32_t bufferSize;        // Ring buffer size
    
    UARTConfig() : instance(UARTInstance::USART1),
                   mode(UARTMode::TX_RX),
                   baudRate(UARTBaudRate::BAUD_115200),
                   dataBits(UARTDataBits::BITS_8),
                   stopBits(UARTStopBits::STOP_1),
                   parity(UARTParity::NONE),
                   flowControl(UARTFlowControl::NONE),
                   oversampling(UARTOversampling::OVERSAMPLING_16),
                   enableDMA(false),
                   enableInterrupt(false),
                   rxTimeout(1000),
                   txTimeout(1000),
                   bufferSize(1024) {}
    
    // Get maximum baud rate based on instance
    uint32_t getMaxBaudRate() const {
        if (instance == UARTInstance::USART1 || instance == UARTInstance::USART6) {
            return 11250000;  // 11.25 Mbps
        } else {
            return 5620000;   // 5.62 Mbps
        }
    }
    
    // Check if baud rate is valid
    bool isValidBaudRate() const {
        return static_cast<uint32_t>(baudRate) <= getMaxBaudRate();
    }
};

//=============================================================================
// UART Message Structure
//=============================================================================
struct UARTMessage {
    std::vector<uint8_t> data;
    uint32_t timestamp;
    bool completed;
    bool error;
    
    UARTMessage() : timestamp(0), completed(false), error(false) {}
    UARTMessage(const std::vector<uint8_t>& d) : data(d), timestamp(0), completed(false), error(false) {}
};

//=============================================================================
// UART Statistics
//=============================================================================
struct UARTStatistics {
    uint64_t bytesTransmitted;
    uint64_t bytesReceived;
    uint32_t txErrors;
    uint32_t rxErrors;
    uint32_t framingErrors;
    uint32_t parityErrors;
    uint32_t overrunErrors;
    uint32_t bufferOverflows;
    uint32_t timeouts;
    
    UARTStatistics() : bytesTransmitted(0), bytesReceived(0), txErrors(0), rxErrors(0),
                       framingErrors(0), parityErrors(0), overrunErrors(0),
                       bufferOverflows(0), timeouts(0) {}
    
    void reset() {
        bytesTransmitted = 0; bytesReceived = 0; txErrors = 0; rxErrors = 0;
        framingErrors = 0; parityErrors = 0; overrunErrors = 0;
        bufferOverflows = 0; timeouts = 0;
    }
};

//=============================================================================
// UART Peripheral Base Class
//=============================================================================
class UARTPeripheral {
protected:
    UARTInstance instance;
    UARTConfig config;
    bool initialized;
    bool enabled;
    
    // TX/RX buffers
    std::queue<uint8_t> txBuffer;
    std::queue<uint8_t> rxBuffer;
    std::vector<uint8_t> rxRingBuffer;
    size_t rxHead;
    size_t rxTail;
    
    // DMA support
    bool dmaTxEnabled;
    bool dmaRxEnabled;
    uint8_t dmaTxStream;
    uint8_t dmaRxStream;
    
    // Callbacks
    std::function<void()> txCompleteCallback;
    std::function<void()> rxCompleteCallback;
    std::function<void(uint8_t)> dataReceivedCallback;
    std::function<void(const std::vector<uint8_t>&)> messageReceivedCallback;
    std::function<void(uint32_t)> errorCallback;
    
    // Statistics
    UARTStatistics statistics;
    
    // Mutex for thread safety
    std::mutex txMutex;
    std::mutex rxMutex;
    
    // Helper methods
    uint32_t getBaseAddress() const;
    std::string getInstanceName() const;
    uint32_t getClockFrequency() const;
    uint32_t calculateBRR() const;
    
public:
    UARTPeripheral(UARTInstance inst);
    virtual ~UARTPeripheral();
    
    // Configuration
    virtual bool init(const UARTConfig& cfg);
    virtual void deinit();
    
    // Control
    virtual bool enable();
    virtual void disable();
    bool isEnabled() const { return enabled; }
    
    // Data transmission
    virtual bool transmit(uint8_t data);
    virtual bool transmit(const uint8_t* data, uint32_t length);
    virtual bool transmit(const std::vector<uint8_t>& data);
    virtual bool transmit(const std::string& str);
    virtual bool transmitAsync(const uint8_t* data, uint32_t length);
    
    // Data reception
    virtual bool receive(uint8_t& data);
    virtual uint32_t receive(uint8_t* buffer, uint32_t maxLength);
    virtual std::vector<uint8_t> receive(uint32_t length);
    virtual std::string receiveString(uint32_t maxLength);
    virtual uint32_t available() const;
    
    // Line operations
    virtual bool sendBreak();
    virtual bool isBreakDetected() const;
    
    // DMA support
    virtual void enableDMA(uint8_t txStream, uint8_t rxStream);
    virtual void disableDMA();
    virtual bool startDMATransmit(const uint8_t* data, uint32_t length);
    virtual bool startDMAReceive(uint8_t* buffer, uint32_t length);
    
    // Status
    virtual bool isTxEmpty() const;
    virtual bool isRxNotEmpty() const;
    virtual bool isTxComplete() const;
    virtual bool isBusy() const;
    
    // Callback registration
    void onTransmitComplete(std::function<void()> callback);
    void onReceiveComplete(std::function<void()> callback);
    void onDataReceived(std::function<void(uint8_t)> callback);
    void onMessageReceived(std::function<void(const std::vector<uint8_t>&)> callback);
    void onError(std::function<void(uint32_t)> callback);
    
    // Statistics
    UARTStatistics getStatistics() const { return statistics; }
    void resetStatistics();
    
    // Simulation functions
    virtual void simulateReceive(uint8_t data);
    virtual void simulateReceive(const uint8_t* data, uint32_t length);
    virtual void simulateReceive(const std::string& str);
    virtual void simulateError(uint32_t errorType);
    
    // Print configuration
    virtual void printConfig() const;
    virtual void printStatus() const;
    
    // Static utility functions
    static std::string getBaudRateString(UARTBaudRate baud);
    static std::string getInstanceString(UARTInstance inst);
};

//=============================================================================
// USART1 Peripheral (High-speed on APB2)
//=============================================================================
class USART1 : public UARTPeripheral {
public:
    USART1() : UARTPeripheral(UARTInstance::USART1) {}
    
    // USART1 specific features
    bool enableSmartCardMode(uint16_t guardTime);
    bool enableLINMode(uint8_t breakLength);
    bool enableIrDAMode(uint32_t lowPowerDivisor);
};

//=============================================================================
// USART2 Peripheral (APB1)
//=============================================================================
class USART2 : public UARTPeripheral {
public:
    USART2() : UARTPeripheral(UARTInstance::USART2) {}
};

//=============================================================================
// USART3 Peripheral (APB1)
//=============================================================================
class USART3 : public UARTPeripheral {
public:
    USART3() : UARTPeripheral(UARTInstance::USART3) {}
};

//=============================================================================
// UART4 Peripheral (APB1)
//=============================================================================
class UART4 : public UARTPeripheral {
public:
    UART4() : UARTPeripheral(UARTInstance::UART4) {}
};

//=============================================================================
// UART5 Peripheral (APB1)
//=============================================================================
class UART5 : public UARTPeripheral {
public:
    UART5() : UARTPeripheral(UARTInstance::UART5) {}
};

//=============================================================================
// USART6 Peripheral (High-speed on APB2)
//=============================================================================
class USART6 : public UARTPeripheral {
public:
    USART6() : UARTPeripheral(UARTInstance::USART6) {}
    
    // USART6 specific features
    bool enableSmartCardMode(uint16_t guardTime);
    bool enableLINMode(uint8_t breakLength);
    bool enableIrDAMode(uint32_t lowPowerDivisor);
};

//=============================================================================
// UART7 Peripheral (APB1)
//=============================================================================
class UART7 : public UARTPeripheral {
public:
    UART7() : UARTPeripheral(UARTInstance::UART7) {}
};

//=============================================================================
// UART8 Peripheral (APB1)
//=============================================================================
class UART8 : public UARTPeripheral {
public:
    UART8() : UARTPeripheral(UARTInstance::UART8) {}
};

//=============================================================================
// UART Manager Class
//=============================================================================
class UARTManager {
private:
    std::map<UARTInstance, std::unique_ptr<UARTPeripheral>> uarts;
    
public:
    UARTManager();
    
    // Get specific UART instance
    USART1* getUSART1();
    USART2* getUSART2();
    USART3* getUSART3();
    UART4* getUART4();
    UART5* getUART5();
    USART6* getUSART6();
    UART7* getUART7();
    UART8* getUART8();
    
    // Get by instance enum
    UARTPeripheral* getUART(UARTInstance inst);
    
    // Initialize all configured UARTs
    void initAll();
    
    // Print status of all UARTs
    void printAllStatus() const;
};

} // namespace STM32F429

#endif  //__UART_CONFIG_H__