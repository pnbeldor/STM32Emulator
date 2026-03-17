#ifndef __CAN_CONFIG_H__
#define __CAN_CONFIG_H__


#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <queue>
#include <map>
#include <chrono>

namespace STM32F429 {

//=============================================================================
// CAN Instance Selection
//=============================================================================
enum class CANInstance {
    CAN1,
    CAN2
};

//=============================================================================
// CAN Mode
//=============================================================================
enum class CANMode {
    NORMAL,
    LOOPBACK,
    SILENT,
    SILENT_LOOPBACK
};

//=============================================================================
// CAN Operating Mode
//=============================================================================
enum class CANOperatingMode {
    INITIALIZATION,
    NORMAL,
    SLEEP,
    STANDBY
};

//=============================================================================
// CAN Baud Rate Prescaler
//=============================================================================
enum class CANBaudRate {
    BAUD_1M = 1000000,
    BAUD_500K = 500000,
    BAUD_250K = 250000,
    BAUD_125K = 125000,
    BAUD_100K = 100000,
    BAUD_50K = 50000,
    BAUD_20K = 20000,
    BAUD_10K = 10000
};

//=============================================================================
// CAN Filter Mode
//=============================================================================
enum class CANFilterMode {
    ID_MASK,        // Identifier mask mode
    ID_LIST         // Identifier list mode
};

//=============================================================================
// CAN Filter Scale
//=============================================================================
enum class CANFilterScale {
    DUAL_16BIT,     // Two 16-bit filters
    SINGLE_32BIT    // One 32-bit filter
};

//=============================================================================
// CAN Filter FIFO Assignment
//=============================================================================
enum class CANFilterFIFO {
    FIFO0,
    FIFO1
};

//=============================================================================
// CAN Filter Configuration
//=============================================================================
struct CANFilterConfig {
    uint8_t filterBank;         // Filter bank number (0-27)
    CANFilterMode mode;         // Filter mode
    CANFilterScale scale;       // Filter scale
    CANFilterFIFO fifo;         // FIFO assignment
    uint32_t id;                 // Identifier
    uint32_t mask;               // Mask (for mask mode)
    bool enable;                 // Filter enable
    
    CANFilterConfig() : filterBank(0), mode(CANFilterMode::ID_MASK),
                        scale(CANFilterScale::SINGLE_32BIT),
                        fifo(CANFilterFIFO::FIFO0),
                        id(0), mask(0xFFFFFFFF), enable(true) {}
};

//=============================================================================
// CAN Message Structure
//=============================================================================
struct CANMessage {
    uint32_t id;                 // Standard ID (11-bit) or Extended ID (29-bit)
    bool isExtended;             // True for extended ID (29-bit), false for standard (11-bit)
    bool isRemote;               // True for remote frame, false for data frame
    uint8_t dlc;                 // Data length code (0-8)
    uint8_t data[8];             // Data bytes
    uint32_t timestamp;          // Message timestamp (in ms or us)
    uint32_t filterMatch;        // Filter that matched this message
    uint8_t fifo;                // FIFO that received this message (0 or 1)
    
    CANMessage() : id(0), isExtended(false), isRemote(false), 
                   dlc(0), timestamp(0), filterMatch(0), fifo(0) {
        for (int i = 0; i < 8; i++) data[i] = 0;
    }
    
    // Constructor for standard message
    CANMessage(uint32_t identifier, uint8_t length, const uint8_t* bytes) 
        : id(identifier), isExtended(false), isRemote(false), 
          dlc(length > 8 ? 8 : length), timestamp(0), filterMatch(0), fifo(0) {
        for (int i = 0; i < dlc; i++) data[i] = bytes[i];
        for (int i = dlc; i < 8; i++) data[i] = 0;
    }
    
    // Constructor for extended message
    CANMessage(uint32_t identifier, bool extended, uint8_t length, const uint8_t* bytes)
        : id(identifier), isExtended(extended), isRemote(false),
          dlc(length > 8 ? 8 : length), timestamp(0), filterMatch(0), fifo(0) {
        for (int i = 0; i < dlc; i++) data[i] = bytes[i];
        for (int i = dlc; i < 8; i++) data[i] = 0;
    }
    
    // Constructor for remote frame
    CANMessage(uint32_t identifier, bool extended, uint8_t length)
        : id(identifier), isExtended(extended), isRemote(true),
          dlc(length > 8 ? 8 : length), timestamp(0), filterMatch(0), fifo(0) {
        for (int i = 0; i < 8; i++) data[i] = 0;
    }
    
    // Convert to string representation
    std::string toString() const {
        std::string result = "ID:0x";
        char hex[16];
        sprintf(hex, "%X", id);
        result += hex;
        result += isExtended ? " (EXT)" : " (STD)";
        result += isRemote ? " RTR" : "";
        result += " DLC:" + std::to_string(dlc);
        if (!isRemote) {
            result += " DATA:";
            for (int i = 0; i < dlc; i++) {
                sprintf(hex, "%02X", data[i]);
                result += hex;
                if (i < dlc - 1) result += " ";
            }
        }
        return result;
    }
};

//=============================================================================
// CAN Mailbox Structure
//=============================================================================
struct CANMailbox {
    enum class State {
        EMPTY,
        PENDING,
        TRANSMITTING,
        COMPLETED,
        ABORTED
    };
    
    State state;
    CANMessage message;
    uint32_t requestTime;
    uint32_t transmitTime;
    uint8_t mailboxIndex;
    uint8_t abortRequested;
    
    CANMailbox() : state(State::EMPTY), requestTime(0), 
                   transmitTime(0), mailboxIndex(0), abortRequested(0) {}
};

//=============================================================================
// CAN Error Counters
//=============================================================================
struct CANErrorCounters {
    uint8_t txErrorCount;       // Transmit error counter
    uint8_t rxErrorCount;       // Receive error counter
    uint8_t lastErrorCode;      // Last error code
    
    CANErrorCounters() : txErrorCount(0), rxErrorCount(0), lastErrorCode(0) {}
};

//=============================================================================
// CAN Timing Parameters
//=============================================================================
struct CANTiming {
    uint32_t baudRate;           // Target baud rate in Hz
    uint8_t prescaler;           // Baud rate prescaler (1-1024)
    uint8_t syncJumpWidth;       // Synchronization jump width (1-4)
    uint8_t timeSeg1;            // Time segment 1 (1-16)
    uint8_t timeSeg2;            // Time segment 2 (1-8)
    bool tripleSampling;         // Triple sampling mode
    
    CANTiming() : baudRate(500000), prescaler(4), syncJumpWidth(1),
                  timeSeg1(13), timeSeg2(2), tripleSampling(false) {}
    
    // Calculate total time quanta per bit
    uint8_t getTimeQuantaPerBit() const {
        return 1 + syncJumpWidth + timeSeg1 + timeSeg2;
    }
    
    // Calculate actual baud rate based on PCLK1
    uint32_t getActualBaudRate(uint32_t pclk1) const {
        return pclk1 / (prescaler * getTimeQuantaPerBit());
    }
};

//=============================================================================
// CAN Statistics
//=============================================================================
struct CANStatistics {
    uint32_t txMessages;          // Transmitted messages
    uint32_t rxMessages;          // Received messages
    uint32_t txErrors;            // Transmit errors
    uint32_t rxErrors;            // Receive errors
    uint32_t busOffEvents;        // Bus-off events
    uint32_t lostMessages;        // Lost messages (FIFO overflow)
    uint32_t arbitrationLost;     // Arbitration lost count
    uint32_t overrunErrors;       // Overrun errors
    uint32_t passiveErrors;       // Error passive events
    uint32_t warningErrors;       // Error warning events
    
    CANStatistics() : txMessages(0), rxMessages(0), txErrors(0), rxErrors(0),
                      busOffEvents(0), lostMessages(0), arbitrationLost(0),
                      overrunErrors(0), passiveErrors(0), warningErrors(0) {}
    
    void reset() {
        txMessages = 0; rxMessages = 0; txErrors = 0; rxErrors = 0;
        busOffEvents = 0; lostMessages = 0; arbitrationLost = 0;
        overrunErrors = 0; passiveErrors = 0; warningErrors = 0;
    }
};

//=============================================================================
// CAN Configuration Structure
//=============================================================================
struct CANConfig {
    CANInstance instance;
    CANMode mode;
    CANBaudRate baudRate;
    uint32_t pclk1;               // APB1 clock frequency (for timing calculation)
    bool autoBusOff;              // Automatic bus-off management
    bool autoWakeUp;              // Automatic wake-up mode
    bool autoRetransmission;      // Automatic retransmission
    bool receiveFifoLocked;       // Receive FIFO locked mode
    bool transmitFifoPriority;    // Transmit FIFO priority
    CANTiming customTiming;       // Custom timing (if auto calculation not used)
    
    CANConfig() : instance(CANInstance::CAN1), mode(CANMode::NORMAL),
                  baudRate(CANBaudRate::BAUD_500K), pclk1(45000000),
                  autoBusOff(true), autoWakeUp(false), autoRetransmission(true),
                  receiveFifoLocked(false), transmitFifoPriority(false) {}
    
    // Calculate timing parameters based on baud rate and PCLK1
    CANTiming calculateTiming() const {
        CANTiming timing;
        timing.baudRate = static_cast<uint32_t>(baudRate);
        
        // Simplified timing calculation for bxCAN
        // In real implementation, this would use more sophisticated algorithm
        uint32_t targetTimeQuanta = pclk1 / static_cast<uint32_t>(baudRate);
        
        if (targetTimeQuanta >= 16 && targetTimeQuanta <= 25) {
            timing.prescaler = 1;
            timing.timeSeg1 = 13;
            timing.timeSeg2 = 2;
        } else if (targetTimeQuanta >= 32 && targetTimeQuanta <= 50) {
            timing.prescaler = 2;
            timing.timeSeg1 = 13;
            timing.timeSeg2 = 2;
        } else if (targetTimeQuanta >= 64 && targetTimeQuanta <= 100) {
            timing.prescaler = 4;
            timing.timeSeg1 = 13;
            timing.timeSeg2 = 2;
        } else if (targetTimeQuanta >= 128 && targetTimeQuanta <= 200) {
            timing.prescaler = 8;
            timing.timeSeg1 = 13;
            timing.timeSeg2 = 2;
        } else {
            // Default
            timing.prescaler = 4;
            timing.timeSeg1 = 13;
            timing.timeSeg2 = 2;
        }
        
        return timing;
    }
};

//=============================================================================
// CAN Peripheral Class
//=============================================================================
class CANPeripheral {
private:
    CANInstance instance;
    CANConfig config;
    bool initialized;
    bool enabled;
    CANOperatingMode currentMode;
    
    // Mailboxes (3 transmit mailboxes)
    CANMailbox txMailboxes[3];
    
    // Receive FIFOs (2 FIFOs, each 3 messages deep)
    std::queue<CANMessage> rxFIFO0;
    std::queue<CANMessage> rxFIFO1;
    static constexpr size_t FIFO_DEPTH = 3;
    
    // Filters (28 filter banks)
    struct FilterBank {
        CANFilterConfig config;
        bool active;
        uint32_t filterIdHigh;
        uint32_t filterIdLow;
        
        FilterBank() : active(false), filterIdHigh(0), filterIdLow(0) {}
    };
    
    FilterBank filterBanks[28];
    
    // Error management
    CANErrorCounters errorCounters;
    CANStatistics statistics;
    
    // Callbacks
    std::function<void(const CANMessage&)> rxCallback;
    std::function<void(const CANMessage&)> txCallback;
    std::function<void(uint8_t, uint8_t)> errorCallback;
    std::function<void()> busOffCallback;
    std::function<void()> wakeupCallback;
    std::function<void()> sleepCallback;
    
    // Timing
    std::chrono::steady_clock::time_point startTime;
    
    // Helper methods
    uint32_t getBaseAddress() const;
    std::string getInstanceName() const;
    uint32_t getCurrentTimestamp() const;
    bool filterMatches(const CANMessage& msg, const FilterBank& filter) const;
    
public:
    CANPeripheral(CANInstance inst);
    ~CANPeripheral();
    
    // Configuration
    bool init(const CANConfig& cfg);
    void deinit();
    bool configureFilters(const std::vector<CANFilterConfig>& filters);
    bool configureFilter(const CANFilterConfig& filter);
    bool enableFilter(uint8_t filterBank, bool enable);
    
    // Control
    bool enable();
    void disable();
    bool isEnabled() const { return enabled; }
    CANOperatingMode getMode() const { return currentMode; }
    bool enterSleepMode();
    bool wakeUp();
    
    // Message transmission
    bool transmit(const CANMessage& message);
    bool transmitAsync(const CANMessage& message);
    bool transmit(const std::vector<CANMessage>& messages);
    bool abortTransmit(uint8_t mailbox);
    uint8_t getTxMailboxCount() const;
    bool isTxComplete(uint8_t mailbox) const;
    
    // Message reception
    bool receive(CANMessage& message, uint8_t fifo = 0);
    bool peek(CANMessage& message, uint8_t fifo = 0) const;
    uint8_t getRxFIFOLevel(uint8_t fifo = 0) const;
    void releaseFIFO(uint8_t fifo = 0);
    void flushFIFO(uint8_t fifo = 0);
    
    // Filter management
    uint8_t getFreeFilterBank() const;
    bool setFilter(uint8_t bank, uint32_t id, uint32_t mask, 
                   CANFilterFIFO fifo = CANFilterFIFO::FIFO0);
    bool setFilterList(uint8_t bank, uint16_t id1, uint16_t id2,
                       CANFilterFIFO fifo = CANFilterFIFO::FIFO0);
    
    // Status and error
    CANErrorCounters getErrorCounters() const { return errorCounters; }
    CANStatistics getStatistics() const { return statistics; }
    void resetStatistics();
    bool isBusOff() const;
    bool isErrorPassive() const;
    bool isErrorWarning() const;
    uint32_t getLastErrorCode() const { return errorCounters.lastErrorCode; }
    
    // Callback registration
    void onReceive(std::function<void(const CANMessage&)> callback);
    void onTransmit(std::function<void(const CANMessage&)> callback);
    void onError(std::function<void(uint8_t, uint8_t)> callback);
    void onBusOff(std::function<void()> callback);
    void onWakeUp(std::function<void()> callback);
    void onSleep(std::function<void()> callback);
    
    // Simulation functions for testing
    void simulateMessage(const CANMessage& message, uint8_t fifo = 0);
    void simulateBusOff();
    void simulateError();
    
    // Print configuration
    void printConfig() const;
    void printStatus() const;
    void printStatistics() const;
    
    // Utility functions
    static std::string messageToString(const CANMessage& msg);
    static uint32_t idToStandard(uint32_t id) { return id & 0x7FF; }
    static uint32_t idToExtended(uint32_t id) { return id & 0x1FFFFFFF; }
    
    // Bus monitoring
    void enableMonitor(bool enable);
    bool isMonitorEnabled() const;
    
    // Test functions
    void runLoopbackTest();
    bool testCommunication(CANPeripheral& other);
    
    // Timestamp management
    void setTimestampSource(uint32_t source);  // 0: timer, 1: external
    uint32_t getTimestamp() const { return getCurrentTimestamp(); }
    
private:
    void processInterrupts();
    void updateErrorState();
    bool selectTxMailbox(CANMessage& msg, uint8_t& mailbox);
    void handleTxComplete(uint8_t mailbox);
    void handleRxFIFO(uint8_t fifo);
    void handleError(uint32_t errorCode);
    
    // Register simulation
    uint32_t readRegister(uint32_t reg);
    void writeRegister(uint32_t reg, uint32_t value);
};

} // namespace STM32F429


#endif  //__CAN_CONFIG_H__