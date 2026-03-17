#include "CANConfig.h"

#include <algorithm>
#include <thread>
#include <random>

namespace STM32F429 {

//=============================================================================
// CANPeripheral Implementation
//=============================================================================

CANPeripheral::CANPeripheral(CANInstance inst) 
    : instance(inst),
      initialized(false),
      enabled(false),
      currentMode(CANOperatingMode::INITIALIZATION) {
    
    // Initialize filter banks
    for (int i = 0; i < 28; i++) {
        filterBanks[i].active = false;
    }
    
    // Initialize mailboxes
    for (int i = 0; i < 3; i++) {
        txMailboxes[i].state = CANMailbox::State::EMPTY;
        txMailboxes[i].mailboxIndex = i;
    }
    
    startTime = std::chrono::steady_clock::now();
}

CANPeripheral::~CANPeripheral() {
    deinit();
}

uint32_t CANPeripheral::getBaseAddress() const {
    switch(instance) {
        case CANInstance::CAN1: return 0x40006400;
        case CANInstance::CAN2: return 0x40006800;
        default: return 0;
    }
}

std::string CANPeripheral::getInstanceName() const {
    switch(instance) {
        case CANInstance::CAN1: return "CAN1";
        case CANInstance::CAN2: return "CAN2";
        default: return "Unknown";
    }
}

uint32_t CANPeripheral::getCurrentTimestamp() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return static_cast<uint32_t>(duration.count());
}

bool CANPeripheral::init(const CANConfig& cfg) {
    config = cfg;
    
    // Calculate timing if not using custom
    if (cfg.customTiming.baudRate == 0) {
        config.customTiming = config.calculateTiming();
    }
    
    initialized = true;
    currentMode = CANOperatingMode::INITIALIZATION;
    
    std::cout << "Initializing " << getInstanceName() << std::endl;
    std::cout << "  Mode: ";
    switch(config.mode) {
        case CANMode::NORMAL: std::cout << "NORMAL"; break;
        case CANMode::LOOPBACK: std::cout << "LOOPBACK"; break;
        case CANMode::SILENT: std::cout << "SILENT"; break;
        case CANMode::SILENT_LOOPBACK: std::cout << "SILENT_LOOPBACK"; break;
    }
    std::cout << std::endl;
    std::cout << "  Baud Rate: " << static_cast<int>(config.baudRate) / 1000 << " kbps" << std::endl;
    std::cout << "  Actual Baud: " << config.customTiming.getActualBaudRate(config.pclk1) / 1000 << " kbps" << std::endl;
    std::cout << "  Prescaler: " << (int)config.customTiming.prescaler << std::endl;
    std::cout << "  Time Seg1: " << (int)config.customTiming.timeSeg1 << std::endl;
    std::cout << "  Time Seg2: " << (int)config.customTiming.timeSeg2 << std::endl;
    
    return true;
}

void CANPeripheral::deinit() {
    disable();
    initialized = false;
    std::cout << "Deinitialized " << getInstanceName() << std::endl;
}

bool CANPeripheral::configureFilters(const std::vector<CANFilterConfig>& filters) {
    for (const auto& filter : filters) {
        if (!configureFilter(filter)) {
            return false;
        }
    }
    return true;
}

bool CANPeripheral::configureFilter(const CANFilterConfig& filter) {
    if (filter.filterBank >= 28) {
        std::cerr << "Error: Invalid filter bank " << (int)filter.filterBank << std::endl;
        return false;
    }
    
    filterBanks[filter.filterBank].config = filter;
    filterBanks[filter.filterBank].active = filter.enable;
    
    // Configure based on scale
    if (filter.scale == CANFilterScale::SINGLE_32BIT) {
        filterBanks[filter.filterBank].filterIdHigh = filter.id;
        filterBanks[filter.filterBank].filterIdLow = filter.mask;
    } else {
        // Dual 16-bit mode
        filterBanks[filter.filterBank].filterIdHigh = filter.id & 0xFFFF;
        filterBanks[filter.filterBank].filterIdLow = (filter.id >> 16) & 0xFFFF;
    }
    
    std::cout << getInstanceName() << " Filter Bank " << (int)filter.filterBank 
              << " configured (FIFO" << (filter.fifo == CANFilterFIFO::FIFO0 ? "0" : "1") << ")" << std::endl;
    
    return true;
}

bool CANPeripheral::enableFilter(uint8_t filterBank, bool enable) {
    if (filterBank >= 28) return false;
    
    filterBanks[filterBank].active = enable;
    filterBanks[filterBank].config.enable = enable;
    
    std::cout << getInstanceName() << " Filter Bank " << (int)filterBank 
              << (enable ? " enabled" : " disabled") << std::endl;
    
    return true;
}

bool CANPeripheral::enable() {
    if (!initialized) {
        std::cerr << "Error: " << getInstanceName() << " not initialized" << std::endl;
        return false;
    }
    
    enabled = true;
    currentMode = CANOperatingMode::NORMAL;
    std::cout << "Enabled " << getInstanceName() << std::endl;
    return true;
}

void CANPeripheral::disable() {
    enabled = false;
    currentMode = CANOperatingMode::SLEEP;
    std::cout << "Disabled " << getInstanceName() << std::endl;
}

bool CANPeripheral::enterSleepMode() {
    if (!enabled) return false;
    
    currentMode = CANOperatingMode::SLEEP;
    std::cout << getInstanceName() << " entered sleep mode" << std::endl;
    
    if (sleepCallback) {
        sleepCallback();
    }
    
    return true;
}

bool CANPeripheral::wakeUp() {
    if (!enabled) return false;
    
    currentMode = CANOperatingMode::NORMAL;
    std::cout << getInstanceName() << " woke up" << std::endl;
    
    if (wakeupCallback) {
        wakeupCallback();
    }
    
    return true;
}

bool CANPeripheral::transmit(const CANMessage& message) {
    if (!enabled || currentMode != CANOperatingMode::NORMAL) {
        statistics.txErrors++;
        return false;
    }
    
    uint8_t mailbox;
    if (!selectTxMailbox(const_cast<CANMessage&>(message), mailbox)) {
        std::cerr << getInstanceName() << " No free transmit mailbox" << std::endl;
        statistics.txErrors++;
        return false;
    }
    
    // Simulate transmission time
    int bitTime = 1; // in microseconds (simplified)
    int totalBits = 1 + 12 + (message.isExtended ? 32 : 18) + message.dlc * 8 + 16;
    int transmitTimeUs = totalBits * bitTime;
    
    std::this_thread::sleep_for(std::chrono::microseconds(transmitTimeUs));
    
    txMailboxes[mailbox].state = CANMailbox::State::COMPLETED;
    txMailboxes[mailbox].transmitTime = getCurrentTimestamp();
    
    statistics.txMessages++;
    
    std::cout << getInstanceName() << " Transmitted: " << message.toString() << std::endl;
    
    if (txCallback) {
        txCallback(message);
    }
    
    // In loopback mode, also receive the message
    if (config.mode == CANMode::LOOPBACK || config.mode == CANMode::SILENT_LOOPBACK) {
        simulateMessage(message);
    }
    
    return true;
}

bool CANPeripheral::transmitAsync(const CANMessage& message) {
    // Simplified async transmission (just queue and return)
    return transmit(message);
}

bool CANPeripheral::transmit(const std::vector<CANMessage>& messages) {
    bool allSuccess = true;
    for (const auto& msg : messages) {
        if (!transmit(msg)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool CANPeripheral::abortTransmit(uint8_t mailbox) {
    if (mailbox >= 3) return false;
    
    if (txMailboxes[mailbox].state == CANMailbox::State::PENDING ||
        txMailboxes[mailbox].state == CANMailbox::State::TRANSMITTING) {
        txMailboxes[mailbox].state = CANMailbox::State::ABORTED;
        txMailboxes[mailbox].abortRequested = 1;
        return true;
    }
    
    return false;
}

uint8_t CANPeripheral::getTxMailboxCount() const {
    uint8_t count = 0;
    for (int i = 0; i < 3; i++) {
        if (txMailboxes[i].state == CANMailbox::State::EMPTY) {
            count++;
        }
    }
    return count;
}

bool CANPeripheral::isTxComplete(uint8_t mailbox) const {
    if (mailbox >= 3) return false;
    return txMailboxes[mailbox].state == CANMailbox::State::COMPLETED;
}

bool CANPeripheral::receive(CANMessage& message, uint8_t fifo) {
    std::queue<CANMessage>* queue = (fifo == 0) ? &rxFIFO0 : &rxFIFO1;
    
    if (queue->empty()) {
        return false;
    }
    
    message = queue->front();
    queue->pop();
    
    statistics.rxMessages++;
    
    return true;
}

bool CANPeripheral::peek(CANMessage& message, uint8_t fifo) const {
    const std::queue<CANMessage>* queue = (fifo == 0) ? &rxFIFO0 : &rxFIFO1;
    
    if (queue->empty()) {
        return false;
    }
    
    message = queue->front();
    return true;
}

uint8_t CANPeripheral::getRxFIFOLevel(uint8_t fifo) const {
    const std::queue<CANMessage>* queue = (fifo == 0) ? &rxFIFO0 : &rxFIFO1;
    return static_cast<uint8_t>(queue->size());
}

void CANPeripheral::releaseFIFO(uint8_t fifo) {
    std::queue<CANMessage>* queue = (fifo == 0) ? &rxFIFO0 : &rxFIFO1;
    
    if (!queue->empty()) {
        queue->pop();
    }
}

void CANPeripheral::flushFIFO(uint8_t fifo) {
    std::queue<CANMessage>* queue = (fifo == 0) ? &rxFIFO0 : &rxFIFO1;
    
    while (!queue->empty()) {
        queue->pop();
    }
}

uint8_t CANPeripheral::getFreeFilterBank() const {
    for (int i = 0; i < 28; i++) {
        if (!filterBanks[i].active) {
            return i;
        }
    }
    return 0xFF; // No free filter
}

bool CANPeripheral::setFilter(uint8_t bank, uint32_t id, uint32_t mask, CANFilterFIFO fifo) {
    if (bank >= 28) return false;
    
    CANFilterConfig filter;
    filter.filterBank = bank;
    filter.mode = CANFilterMode::ID_MASK;
    filter.scale = CANFilterScale::SINGLE_32BIT;
    filter.fifo = fifo;
    filter.id = id;
    filter.mask = mask;
    filter.enable = true;
    
    return configureFilter(filter);
}

bool CANPeripheral::setFilterList(uint8_t bank, uint16_t id1, uint16_t id2, CANFilterFIFO fifo) {
    if (bank >= 28) return false;
    
    CANFilterConfig filter;
    filter.filterBank = bank;
    filter.mode = CANFilterMode::ID_LIST;
    filter.scale = CANFilterScale::DUAL_16BIT;
    filter.fifo = fifo;
    filter.id = (static_cast<uint32_t>(id2) << 16) | id1;
    filter.mask = 0;
    filter.enable = true;
    
    return configureFilter(filter);
}

void CANPeripheral::resetStatistics() {
    statistics.reset();
    errorCounters.txErrorCount = 0;
    errorCounters.rxErrorCount = 0;
}

bool CANPeripheral::isBusOff() const {
    return errorCounters.txErrorCount > 255;
}

bool CANPeripheral::isErrorPassive() const {
    return (errorCounters.txErrorCount > 127) || (errorCounters.rxErrorCount > 127);
}

bool CANPeripheral::isErrorWarning() const {
    return (errorCounters.txErrorCount > 95) || (errorCounters.rxErrorCount > 95);
}

void CANPeripheral::onReceive(std::function<void(const CANMessage&)> callback) {
    rxCallback = callback;
}

void CANPeripheral::onTransmit(std::function<void(const CANMessage&)> callback) {
    txCallback = callback;
}

void CANPeripheral::onError(std::function<void(uint8_t, uint8_t)> callback) {
    errorCallback = callback;
}

void CANPeripheral::onBusOff(std::function<void()> callback) {
    busOffCallback = callback;
}

void CANPeripheral::onWakeUp(std::function<void()> callback) {
    wakeupCallback = callback;
}

void CANPeripheral::onSleep(std::function<void()> callback) {
    sleepCallback = callback;
}

bool CANPeripheral::filterMatches(const CANMessage& msg, const FilterBank& filter) const {
    if (!filter.active) return true; // Inactive filters pass all
    
    const auto& cfg = filter.config;
    
    if (cfg.mode == CANFilterMode::ID_MASK) {
        // Mask mode
        uint32_t maskedId = msg.id & cfg.mask;
        uint32_t maskedFilter = cfg.id & cfg.mask;
        return maskedId == maskedFilter;
    } else {
        // List mode
        if (cfg.scale == CANFilterScale::SINGLE_32BIT) {
            return (msg.id == cfg.id);
        } else {
            // Dual 16-bit
            uint16_t idLow = cfg.id & 0xFFFF;
            uint16_t idHigh = (cfg.id >> 16) & 0xFFFF;
            
            if (msg.isExtended) {
                // For extended IDs in list mode, we need to handle differently
                return (msg.id == static_cast<uint32_t>(idLow)) || 
                       (msg.id == static_cast<uint32_t>(idHigh));
            } else {
                uint16_t stdId = msg.id & 0x7FF;
                return (stdId == idLow) || (stdId == idHigh);
            }
        }
    }
}

void CANPeripheral::simulateMessage(const CANMessage& message, uint8_t fifo) {
    if (!enabled) return;
    
    // Check filters
    bool accepted = false;
    uint8_t matchingFilter = 0;
    
    for (int i = 0; i < 28; i++) {
        if (filterBanks[i].active && filterMatches(message, filterBanks[i])) {
            accepted = true;
            matchingFilter = i;
            fifo = (filterBanks[i].config.fifo == CANFilterFIFO::FIFO0) ? 0 : 1;
            break;
        }
    }
    
    // Default accept if no filters active
    if (!accepted) {
        for (int i = 0; i < 28; i++) {
            if (filterBanks[i].active) {
                return; // At least one filter active but none matched -> reject
            }
        }
        // No filters active -> accept all
        accepted = true;
    }
    
    if (!accepted) return;
    
    CANMessage msg = message;
    msg.filterMatch = matchingFilter;
    msg.fifo = fifo;
    msg.timestamp = getCurrentTimestamp();
    
    std::queue<CANMessage>* queue = (fifo == 0) ? &rxFIFO0 : &rxFIFO1;
    
    if (queue->size() >= FIFO_DEPTH) {
        statistics.lostMessages++;
        if (config.receiveFifoLocked) {
            return; // Locked mode, don't overwrite
        } else {
            queue->pop(); // Overwrite oldest
        }
    }
    
    queue->push(msg);
    
    std::cout << getInstanceName() << " Received: " << msg.toString() 
              << " (FIFO" << (int)fifo << ")" << std::endl;
    
    if (rxCallback) {
        rxCallback(msg);
    }
}

void CANPeripheral::simulateBusOff() {
    errorCounters.txErrorCount = 256;
    statistics.busOffEvents++;
    
    std::cout << getInstanceName() << " BUS OFF" << std::endl;
    
    if (busOffCallback) {
        busOffCallback();
    }
}

void CANPeripheral::simulateError() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> errDist(1, 100);
    
    uint8_t errorCode = errDist(gen) % 5;
    errorCounters.lastErrorCode = errorCode;
    
    std::cout << getInstanceName() << " Error: Code " << (int)errorCode << std::endl;
    
    if (errorCallback) {
        errorCallback(errorCode, errorCounters.txErrorCount);
    }
}

void CANPeripheral::enableMonitor(bool enable) {
    // In monitor mode, CAN receives but does not transmit
    std::cout << getInstanceName() << " Monitor mode " << (enable ? "enabled" : "disabled") << std::endl;
}

bool CANPeripheral::isMonitorEnabled() const {
    return false; // Simplified
}

void CANPeripheral::runLoopbackTest() {
    std::cout << "\n=== " << getInstanceName() << " Loopback Test ===" << std::endl;
    
    // Save current mode
    CANMode originalMode = config.mode;
    config.mode = CANMode::LOOPBACK;
    
    // Test message
    uint8_t testData[] = {0xDE, 0xAD, 0xBE, 0xEF};
    CANMessage testMsg(0x123, false, 4, testData);
    
    std::cout << "Sending: " << testMsg.toString() << std::endl;
    
    if (transmit(testMsg)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        CANMessage received;
        if (receive(received)) {
            std::cout << "Received: " << received.toString() << std::endl;
            std::cout << "Loopback test PASSED" << std::endl;
        } else {
            std::cout << "Loopback test FAILED (no message received)" << std::endl;
        }
    } else {
        std::cout << "Loopback test FAILED (transmit failed)" << std::endl;
    }
    
    // Restore mode
    config.mode = originalMode;
}

bool CANPeripheral::testCommunication(CANPeripheral& other) {
    std::cout << "\n=== CAN Communication Test between " 
              << getInstanceName() << " and " << other.getInstanceName() << " ===" << std::endl;
    
    // Set up filters
    setFilter(0, 0x123, 0x7FF, CANFilterFIFO::FIFO0);
    other.setFilter(0, 0x456, 0x7FF, CANFilterFIFO::FIFO0);
    
    // Message from this to other
    uint8_t data1[] = {0x11, 0x22, 0x33, 0x44};
    CANMessage msg1(0x123, false, 4, data1);
    
    // Message from other to this
    uint8_t data2[] = {0x55, 0x66, 0x77, 0x88};
    CANMessage msg2(0x456, false, 4, data2);
    
    bool success = true;
    
    // Transmit from this
    if (transmit(msg1)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        CANMessage received;
        if (other.receive(received)) {
            std::cout << "Other received: " << received.toString() << std::endl;
        } else {
            std::cout << "Other failed to receive message" << std::endl;
            success = false;
        }
    } else {
        success = false;
    }
    
    // Transmit from other
    if (other.transmit(msg2)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        CANMessage received;
        if (receive(received)) {
            std::cout << "This received: " << received.toString() << std::endl;
        } else {
            std::cout << "This failed to receive message" << std::endl;
            success = false;
        }
    } else {
        success = false;
    }
    
    std::cout << "Communication test " << (success ? "PASSED" : "FAILED") << std::endl;
    
    return success;
}

void CANPeripheral::printConfig() const {
    std::cout << "\n=== " << getInstanceName() << " Configuration ===" << std::endl;
    std::cout << "Base Address: 0x" << std::hex << getBaseAddress() << std::dec << std::endl;
    std::cout << "Initialized: " << (initialized ? "Yes" : "No") << std::endl;
    std::cout << "Enabled: " << (enabled ? "Yes" : "No") << std::endl;
    std::cout << "Mode: ";
    switch(currentMode) {
        case CANOperatingMode::INITIALIZATION: std::cout << "INITIALIZATION"; break;
        case CANOperatingMode::NORMAL: std::cout << "NORMAL"; break;
        case CANOperatingMode::SLEEP: std::cout << "SLEEP"; break;
        case CANOperatingMode::STANDBY: std::cout << "STANDBY"; break;
    }
    std::cout << std::endl;
    
    std::cout << "CAN Mode: ";
    switch(config.mode) {
        case CANMode::NORMAL: std::cout << "NORMAL"; break;
        case CANMode::LOOPBACK: std::cout << "LOOPBACK"; break;
        case CANMode::SILENT: std::cout << "SILENT"; break;
        case CANMode::SILENT_LOOPBACK: std::cout << "SILENT_LOOPBACK"; break;
    }
    std::cout << std::endl;
    
    std::cout << "Baud Rate: " << static_cast<int>(config.baudRate) / 1000 << " kbps" << std::endl;
    std::cout << "Prescaler: " << (int)config.customTiming.prescaler << std::endl;
    std::cout << "Time Seg1: " << (int)config.customTiming.timeSeg1 << std::endl;
    std::cout << "Time Seg2: " << (int)config.customTiming.timeSeg2 << std::endl;
    std::cout << "Sync Jump Width: " << (int)config.customTiming.syncJumpWidth << std::endl;
    
    // Print active filters
    int activeFilters = 0;
    for (int i = 0; i < 28; i++) {
        if (filterBanks[i].active) activeFilters++;
    }
    std::cout << "Active Filters: " << activeFilters << "/28" << std::endl;
}

void CANPeripheral::printStatus() const {
    std::cout << "\n=== " << getInstanceName() << " Status ===" << std::endl;
    std::cout << "TX Mailboxes: ";
    for (int i = 0; i < 3; i++) {
        switch(txMailboxes[i].state) {
            case CANMailbox::State::EMPTY: std::cout << "E "; break;
            case CANMailbox::State::PENDING: std::cout << "P "; break;
            case CANMailbox::State::TRANSMITTING: std::cout << "T "; break;
            case CANMailbox::State::COMPLETED: std::cout << "C "; break;
            case CANMailbox::State::ABORTED: std::cout << "A "; break;
        }
    }
    std::cout << std::endl;
    
    std::cout << "RX FIFO0: " << rxFIFO0.size() << "/" << FIFO_DEPTH << std::endl;
    std::cout << "RX FIFO1: " << rxFIFO1.size() << "/" << FIFO_DEPTH << std::endl;
    
    std::cout << "Error Counters - TX: " << (int)errorCounters.txErrorCount 
              << " RX: " << (int)errorCounters.rxErrorCount << std::endl;
    
    if (isBusOff()) std::cout << "BUS OFF" << std::endl;
    else if (isErrorPassive()) std::cout << "ERROR PASSIVE" << std::endl;
    else if (isErrorWarning()) std::cout << "ERROR WARNING" << std::endl;
}

void CANPeripheral::printStatistics() const {
    std::cout << "\n=== " << getInstanceName() << " Statistics ===" << std::endl;
    std::cout << "TX Messages: " << statistics.txMessages << std::endl;
    std::cout << "RX Messages: " << statistics.rxMessages << std::endl;
    std::cout << "TX Errors: " << statistics.txErrors << std::endl;
    std::cout << "RX Errors: " << statistics.rxErrors << std::endl;
    std::cout << "Bus Off Events: " << statistics.busOffEvents << std::endl;
    std::cout << "Lost Messages: " << statistics.lostMessages << std::endl;
    std::cout << "Arbitration Lost: " << statistics.arbitrationLost << std::endl;
    std::cout << "Overrun Errors: " << statistics.overrunErrors << std::endl;
}

std::string CANPeripheral::messageToString(const CANMessage& msg) {
    return msg.toString();
}

void CANPeripheral::processInterrupts() {
    // Handle transmit complete interrupts
    for (int i = 0; i < 3; i++) {
        if (txMailboxes[i].state == CANMailbox::State::COMPLETED) {
            handleTxComplete(i);
        }
    }
    
    // Handle receive interrupts
    if (!rxFIFO0.empty() || !rxFIFO1.empty()) {
        if (rxCallback) {
            if (!rxFIFO0.empty()) {
                CANMessage msg = rxFIFO0.front();
                rxCallback(msg);
            }
            if (!rxFIFO1.empty()) {
                CANMessage msg = rxFIFO1.front();
                rxCallback(msg);
            }
        }
    }
    
    // Handle error interrupts
    if (isBusOff() && busOffCallback) {
        busOffCallback();
    }
}

void CANPeripheral::updateErrorState() {
    if (isBusOff()) {
        if (config.autoBusOff) {
            // Auto recover from bus off
            errorCounters.txErrorCount = 0;
            errorCounters.rxErrorCount = 0;
        }
    }
}

bool CANPeripheral::selectTxMailbox(CANMessage& msg, uint8_t& mailbox) {
    // Find empty mailbox
    for (int i = 0; i < 3; i++) {
        if (txMailboxes[i].state == CANMailbox::State::EMPTY) {
            mailbox = i;
            txMailboxes[i].state = CANMailbox::State::PENDING;
            txMailboxes[i].message = msg;
            txMailboxes[i].requestTime = getCurrentTimestamp();
            return true;
        }
    }
    
    // If no empty mailbox and priority mode, check if we can preempt
    if (config.transmitFifoPriority) {
        // Find lowest priority message to preempt (simplified)
        return false;
    }
    
    return false;
}

void CANPeripheral::handleTxComplete(uint8_t mailbox) {
    txMailboxes[mailbox].state = CANMailbox::State::EMPTY;
    // Could trigger callback here
}

void CANPeripheral::handleRxFIFO(uint8_t fifo) {
    // Handle receive FIFO interrupt
}

void CANPeripheral::handleError(uint32_t errorCode) {
    errorCounters.lastErrorCode = errorCode;
    
    if (errorCallback) {
        errorCallback(errorCode, errorCounters.txErrorCount);
    }
}

uint32_t CANPeripheral::readRegister(uint32_t reg) {
    // Simulate register read
    return 0;
}

void CANPeripheral::writeRegister(uint32_t reg, uint32_t value) {
    // Simulate register write
}

} // namespace STM32F429