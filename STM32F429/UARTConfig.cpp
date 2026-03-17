#include "UARTConfig.h"

#include <iomanip>
#include <algorithm>

namespace STM32F429 {

//=============================================================================
// UARTPeripheral Implementation
//=============================================================================

UARTPeripheral::UARTPeripheral(UARTInstance inst) 
    : instance(inst),
      initialized(false),
      enabled(false),
      rxHead(0),
      rxTail(0),
      dmaTxEnabled(false),
      dmaRxEnabled(false),
      dmaTxStream(0),
      dmaRxStream(0) {
    
    // Initialize ring buffer
    rxRingBuffer.resize(1024, 0);
}

UARTPeripheral::~UARTPeripheral() {
    deinit();
}

uint32_t UARTPeripheral::getBaseAddress() const {
    switch(instance) {
        case UARTInstance::USART1: return 0x40011000;
        case UARTInstance::USART2: return 0x40004400;
        case UARTInstance::USART3: return 0x40004800;
        case UARTInstance::UART4: return 0x40004C00;
        case UARTInstance::UART5: return 0x40005000;
        case UARTInstance::USART6: return 0x40011400;
        case UARTInstance::UART7: return 0x40007800;
        case UARTInstance::UART8: return 0x40007C00;
        default: return 0;
    }
}

std::string UARTPeripheral::getInstanceName() const {
    switch(instance) {
        case UARTInstance::USART1: return "USART1";
        case UARTInstance::USART2: return "USART2";
        case UARTInstance::USART3: return "USART3";
        case UARTInstance::UART4: return "UART4";
        case UARTInstance::UART5: return "UART5";
        case UARTInstance::USART6: return "USART6";
        case UARTInstance::UART7: return "UART7";
        case UARTInstance::UART8: return "UART8";
        default: return "Unknown";
    }
}

uint32_t UARTPeripheral::getClockFrequency() const {
    // USART1 and USART6 are on APB2 (up to 90 MHz)
    // Others are on APB1 (up to 45 MHz)
    if (instance == UARTInstance::USART1 || instance == UARTInstance::USART6) {
        return 90000000;  // APB2 max
    } else {
        return 45000000;  // APB1 max
    }
}

uint32_t UARTPeripheral::calculateBRR() const {
    uint32_t pclk = getClockFrequency();
    uint32_t baud = static_cast<uint32_t>(config.baudRate);
    uint32_t oversampling = static_cast<uint32_t>(config.oversampling);
    
    // Calculate baud rate register value
    // BRR = (pclk + (baud * oversampling / 2)) / (baud * oversampling)
    uint32_t brr = (pclk + (baud * oversampling / 2)) / (baud * oversampling);
    
    return brr;
}

bool UARTPeripheral::init(const UARTConfig& cfg) {
    config = cfg;
    
    if (!config.isValidBaudRate()) {
        std::cerr << "Error: Baud rate " << static_cast<int>(cfg.baudRate) 
                  << " exceeds maximum for " << getInstanceName() << std::endl;
        return false;
    }
    
    // Resize ring buffer
    rxRingBuffer.resize(cfg.bufferSize);
    rxHead = 0;
    rxTail = 0;
    
    initialized = true;
    
    std::cout << "Initializing " << getInstanceName() << std::endl;
    std::cout << "  Mode: ";
    switch(config.mode) {
        case UARTMode::TX: std::cout << "TX"; break;
        case UARTMode::RX: std::cout << "RX"; break;
        case UARTMode::TX_RX: std::cout << "TX/RX"; break;
        case UARTMode::HALF_DUPLEX: std::cout << "Half-Duplex"; break;
        case UARTMode::LIN: std::cout << "LIN"; break;
        case UARTMode::IRDA: std::cout << "IrDA"; break;
        case UARTMode::SMART_CARD: std::cout << "Smart Card"; break;
    }
    std::cout << std::endl;
    
    std::cout << "  Baud Rate: " << getBaudRateString(config.baudRate) << std::endl;
    std::cout << "  Data Bits: " << static_cast<int>(config.dataBits) << std::endl;
    std::cout << "  Stop Bits: ";
    switch(config.stopBits) {
        case UARTStopBits::STOP_0_5: std::cout << "0.5"; break;
        case UARTStopBits::STOP_1: std::cout << "1"; break;
        case UARTStopBits::STOP_1_5: std::cout << "1.5"; break;
        case UARTStopBits::STOP_2: std::cout << "2"; break;
    }
    std::cout << std::endl;
    
    std::cout << "  Parity: ";
    switch(config.parity) {
        case UARTParity::NONE: std::cout << "None"; break;
        case UARTParity::EVEN: std::cout << "Even"; break;
        case UARTParity::ODD: std::cout << "Odd"; break;
    }
    std::cout << std::endl;
    
    std::cout << "  Flow Control: ";
    switch(config.flowControl) {
        case UARTFlowControl::NONE: std::cout << "None"; break;
        case UARTFlowControl::RTS: std::cout << "RTS"; break;
        case UARTFlowControl::CTS: std::cout << "CTS"; break;
        case UARTFlowControl::RTS_CTS: std::cout << "RTS/CTS"; break;
    }
    std::cout << std::endl;
    
    std::cout << "  Oversampling: " << static_cast<int>(config.oversampling) << std::endl;
    std::cout << "  BRR Value: 0x" << std::hex << calculateBRR() << std::dec << std::endl;
    
    return true;
}

void UARTPeripheral::deinit() {
    disable();
    initialized = false;
    std::cout << "Deinitialized " << getInstanceName() << std::endl;
}

bool UARTPeripheral::enable() {
    if (!initialized) {
        std::cerr << "Error: " << getInstanceName() << " not initialized" << std::endl;
        return false;
    }
    
    enabled = true;
    std::cout << "Enabled " << getInstanceName() << std::endl;
    return true;
}

void UARTPeripheral::disable() {
    enabled = false;
    std::cout << "Disabled " << getInstanceName() << std::endl;
}

bool UARTPeripheral::transmit(uint8_t data) {
    if (!enabled || (config.mode == UARTMode::RX)) {
        statistics.txErrors++;
        return false;
    }
    
    std::lock_guard<std::mutex> lock(txMutex);
    
    // Simulate transmission time
    uint32_t bitTime = 1000000 / static_cast<uint32_t>(config.baudRate);
    uint32_t totalBits = 1 + static_cast<int>(config.dataBits) + 
                         (config.parity != UARTParity::NONE ? 1 : 0) +
                         (config.stopBits == UARTStopBits::STOP_1 ? 1 : 2);
    uint32_t byteTimeUs = totalBits * bitTime;
    
    std::this_thread::sleep_for(std::chrono::microseconds(byteTimeUs));
    
    txBuffer.push(data);
    statistics.bytesTransmitted++;
    
    std::cout << getInstanceName() << " TX: 0x" << std::hex << (int)data << std::dec;
    if (data >= 32 && data <= 126) {
        std::cout << " '" << (char)data << "'";
    }
    std::cout << std::endl;
    
    if (txCompleteCallback) {
        txCompleteCallback();
    }
    
    return true;
}

bool UARTPeripheral::transmit(const uint8_t* data, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        if (!transmit(data[i])) {
            return false;
        }
    }
    return true;
}

bool UARTPeripheral::transmit(const std::vector<uint8_t>& data) {
    return transmit(data.data(), data.size());
}

bool UARTPeripheral::transmit(const std::string& str) {
    return transmit(reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
}

bool UARTPeripheral::transmitAsync(const uint8_t* data, uint32_t length) {
    // For simulation, just transmit immediately
    return transmit(data, length);
}

bool UARTPeripheral::receive(uint8_t& data) {
    if (!enabled || (config.mode == UARTMode::TX)) {
        statistics.rxErrors++;
        return false;
    }
    
    std::lock_guard<std::mutex> lock(rxMutex);
    
    if (rxBuffer.empty()) {
        return false;
    }
    
    data = rxBuffer.front();
    rxBuffer.pop();
    statistics.bytesReceived++;
    
    return true;
}

uint32_t UARTPeripheral::receive(uint8_t* buffer, uint32_t maxLength) {
    uint32_t count = 0;
    
    for (uint32_t i = 0; i < maxLength; i++) {
        uint8_t data;
        if (receive(data)) {
            buffer[count++] = data;
        } else {
            break;
        }
    }
    
    return count;
}

std::vector<uint8_t> UARTPeripheral::receive(uint32_t length) {
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

std::string UARTPeripheral::receiveString(uint32_t maxLength) {
    std::string result;
    uint8_t data;
    
    for (uint32_t i = 0; i < maxLength; i++) {
        if (receive(data)) {
            if (data == '\r' || data == '\n') {
                break;
            }
            result += static_cast<char>(data);
        } else {
            break;
        }
    }
    
    return result;
}

uint32_t UARTPeripheral::available() const {
    return static_cast<uint32_t>(rxBuffer.size());
}

bool UARTPeripheral::sendBreak() {
    if (!enabled) return false;
    
    std::cout << getInstanceName() << " Sending BREAK" << std::endl;
    // Simulate break condition (10-12 bits low)
    return true;
}

bool UARTPeripheral::isBreakDetected() const {
    return false; // Simplified
}

void UARTPeripheral::enableDMA(uint8_t txStream, uint8_t rxStream) {
    dmaTxEnabled = true;
    dmaRxEnabled = true;
    dmaTxStream = txStream;
    dmaRxStream = rxStream;
    config.enableDMA = true;
    
    std::cout << getInstanceName() << " DMA enabled (TX Stream " 
              << (int)txStream << ", RX Stream " << (int)rxStream << ")" << std::endl;
}

void UARTPeripheral::disableDMA() {
    dmaTxEnabled = false;
    dmaRxEnabled = false;
    config.enableDMA = false;
    
    std::cout << getInstanceName() << " DMA disabled" << std::endl;
}

bool UARTPeripheral::startDMATransmit(const uint8_t* data, uint32_t length) {
    if (!dmaTxEnabled) {
        std::cerr << "Error: DMA TX not enabled for " << getInstanceName() << std::endl;
        return false;
    }
    
    std::cout << getInstanceName() << " DMA transmit: " << length << " bytes" << std::endl;
    return transmit(data, length);
}

bool UARTPeripheral::startDMAReceive(uint8_t* buffer, uint32_t length) {
    if (!dmaRxEnabled) {
        std::cerr << "Error: DMA RX not enabled for " << getInstanceName() << std::endl;
        return false;
    }
    
    std::cout << getInstanceName() << " DMA receive: " << length << " bytes" << std::endl;
    
    // Simulate DMA filling buffer
    for (uint32_t i = 0; i < length && !rxBuffer.empty(); i++) {
        buffer[i] = rxBuffer.front();
        rxBuffer.pop();
    }
    
    return true;
}

bool UARTPeripheral::isTxEmpty() const {
    return txBuffer.empty();
}

bool UARTPeripheral::isRxNotEmpty() const {
    return !rxBuffer.empty();
}

bool UARTPeripheral::isTxComplete() const {
    return txBuffer.empty();
}

bool UARTPeripheral::isBusy() const {
    return !txBuffer.empty();
}

void UARTPeripheral::onTransmitComplete(std::function<void()> callback) {
    txCompleteCallback = callback;
}

void UARTPeripheral::onReceiveComplete(std::function<void()> callback) {
    rxCompleteCallback = callback;
}

void UARTPeripheral::onDataReceived(std::function<void(uint8_t)> callback) {
    dataReceivedCallback = callback;
}

void UARTPeripheral::onMessageReceived(std::function<void(const std::vector<uint8_t>&)> callback) {
    messageReceivedCallback = callback;
}

void UARTPeripheral::onError(std::function<void(uint32_t)> callback) {
    errorCallback = callback;
}

void UARTPeripheral::resetStatistics() {
    statistics.reset();
}

void UARTPeripheral::simulateReceive(uint8_t data) {
    std::lock_guard<std::mutex> lock(rxMutex);
    
    rxBuffer.push(data);
    
    // Ring buffer implementation
    rxRingBuffer[rxHead] = data;
    rxHead = (rxHead + 1) % rxRingBuffer.size();
    if (rxHead == rxTail) {
        // Buffer overflow
        statistics.bufferOverflows++;
        rxTail = (rxTail + 1) % rxRingBuffer.size();
    }
    
    std::cout << getInstanceName() << " RX: 0x" << std::hex << (int)data << std::dec;
    if (data >= 32 && data <= 126) {
        std::cout << " '" << (char)data << "'";
    }
    std::cout << std::endl;
    
    if (dataReceivedCallback) {
        dataReceivedCallback(data);
    }
    
    if (rxCompleteCallback) {
        rxCompleteCallback();
    }
}

void UARTPeripheral::simulateReceive(const uint8_t* data, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        simulateReceive(data[i]);
    }
    
    if (messageReceivedCallback) {
        std::vector<uint8_t> msg(data, data + length);
        messageReceivedCallback(msg);
    }
}

void UARTPeripheral::simulateReceive(const std::string& str) {
    simulateReceive(reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
}

void UARTPeripheral::simulateError(uint32_t errorType) {
    switch(errorType) {
        case 0: // Framing error
            statistics.framingErrors++;
            std::cerr << getInstanceName() << " Framing Error" << std::endl;
            break;
        case 1: // Parity error
            statistics.parityErrors++;
            std::cerr << getInstanceName() << " Parity Error" << std::endl;
            break;
        case 2: // Overrun error
            statistics.overrunErrors++;
            std::cerr << getInstanceName() << " Overrun Error" << std::endl;
            break;
    }
    
    if (errorCallback) {
        errorCallback(errorType);
    }
}

void UARTPeripheral::printConfig() const {
    std::cout << "\n=== " << getInstanceName() << " Configuration ===" << std::endl;
    std::cout << "Base Address: 0x" << std::hex << getBaseAddress() << std::dec << std::endl;
    std::cout << "Initialized: " << (initialized ? "Yes" : "No") << std::endl;
    std::cout << "Enabled: " << (enabled ? "Yes" : "No") << std::endl;
    std::cout << "Baud Rate: " << getBaudRateString(config.baudRate) << std::endl;
    std::cout << "Data Bits: " << static_cast<int>(config.dataBits) << std::endl;
    std::cout << "Parity: ";
    switch(config.parity) {
        case UARTParity::NONE: std::cout << "None"; break;
        case UARTParity::EVEN: std::cout << "Even"; break;
        case UARTParity::ODD: std::cout << "Odd"; break;
    }
    std::cout << std::endl;
}

void UARTPeripheral::printStatus() const {
    std::cout << "\n=== " << getInstanceName() << " Status ===" << std::endl;
    std::cout << "TX Buffer: " << txBuffer.size() << " bytes" << std::endl;
    std::cout << "RX Buffer: " << rxBuffer.size() << " bytes" << std::endl;
    std::cout << "Ring Buffer: " << (rxHead - rxTail + rxRingBuffer.size()) % rxRingBuffer.size() << " bytes" << std::endl;
    std::cout << "Statistics:" << std::endl;
    std::cout << "  TX Bytes: " << statistics.bytesTransmitted << std::endl;
    std::cout << "  RX Bytes: " << statistics.bytesReceived << std::endl;
    std::cout << "  TX Errors: " << statistics.txErrors << std::endl;
    std::cout << "  RX Errors: " << statistics.rxErrors << std::endl;
    std::cout << "  Framing Errors: " << statistics.framingErrors << std::endl;
    std::cout << "  Parity Errors: " << statistics.parityErrors << std::endl;
    std::cout << "  Overrun Errors: " << statistics.overrunErrors << std::endl;
    std::cout << "  Buffer Overflows: " << statistics.bufferOverflows << std::endl;
}

std::string UARTPeripheral::getBaudRateString(UARTBaudRate baud) {
    switch(baud) {
        case UARTBaudRate::BAUD_1200: return "1200";
        case UARTBaudRate::BAUD_2400: return "2400";
        case UARTBaudRate::BAUD_4800: return "4800";
        case UARTBaudRate::BAUD_9600: return "9600";
        case UARTBaudRate::BAUD_19200: return "19200";
        case UARTBaudRate::BAUD_38400: return "38400";
        case UARTBaudRate::BAUD_57600: return "57600";
        case UARTBaudRate::BAUD_115200: return "115200";
        case UARTBaudRate::BAUD_230400: return "230400";
        case UARTBaudRate::BAUD_460800: return "460800";
        case UARTBaudRate::BAUD_921600: return "921600";
        case UARTBaudRate::BAUD_2M: return "2M";
        case UARTBaudRate::BAUD_3M: return "3M";
        case UARTBaudRate::BAUD_4M: return "4M";
        case UARTBaudRate::BAUD_5M: return "5M";
        case UARTBaudRate::BAUD_6M: return "6M";
        case UARTBaudRate::BAUD_7M: return "7M";
        case UARTBaudRate::BAUD_8M: return "8M";
        case UARTBaudRate::BAUD_9M: return "9M";
        case UARTBaudRate::BAUD_10M: return "10M";
        case UARTBaudRate::BAUD_11M25: return "11.25M";
        default: return "Unknown";
    }
}

std::string UARTPeripheral::getInstanceString(UARTInstance inst) {
    switch(inst) {
        case UARTInstance::USART1: return "USART1";
        case UARTInstance::USART2: return "USART2";
        case UARTInstance::USART3: return "USART3";
        case UARTInstance::UART4: return "UART4";
        case UARTInstance::UART5: return "UART5";
        case UARTInstance::USART6: return "USART6";
        case UARTInstance::UART7: return "UART7";
        case UARTInstance::UART8: return "UART8";
        default: return "Unknown";
    }
}

//=============================================================================
// USART1 Implementation
//=============================================================================

bool USART1::enableSmartCardMode(uint16_t guardTime) {
    if (!enabled) return false;
    std::cout << "USART1 Smart Card mode enabled, guard time: " << guardTime << std::endl;
    return true;
}

bool USART1::enableLINMode(uint8_t breakLength) {
    if (!enabled) return false;
    std::cout << "USART1 LIN mode enabled, break length: " << (int)breakLength << std::endl;
    return true;
}

bool USART1::enableIrDAMode(uint32_t lowPowerDivisor) {
    if (!enabled) return false;
    std::cout << "USART1 IrDA mode enabled, low power divisor: " << lowPowerDivisor << std::endl;
    return true;
}

//=============================================================================
// USART6 Implementation
//=============================================================================

bool USART6::enableSmartCardMode(uint16_t guardTime) {
    if (!enabled) return false;
    std::cout << "USART6 Smart Card mode enabled, guard time: " << guardTime << std::endl;
    return true;
}

bool USART6::enableLINMode(uint8_t breakLength) {
    if (!enabled) return false;
    std::cout << "USART6 LIN mode enabled, break length: " << (int)breakLength << std::endl;
    return true;
}

bool USART6::enableIrDAMode(uint32_t lowPowerDivisor) {
    if (!enabled) return false;
    std::cout << "USART6 IrDA mode enabled, low power divisor: " << lowPowerDivisor << std::endl;
    return true;
}

//=============================================================================
// UARTManager Implementation
//=============================================================================

UARTManager::UARTManager() {
    uarts[UARTInstance::USART1] = std::make_unique<USART1>();
    uarts[UARTInstance::USART2] = std::make_unique<USART2>();
    uarts[UARTInstance::USART3] = std::make_unique<USART3>();
    uarts[UARTInstance::UART4] = std::make_unique<UART4>();
    uarts[UARTInstance::UART5] = std::make_unique<UART5>();
    uarts[UARTInstance::USART6] = std::make_unique<USART6>();
    uarts[UARTInstance::UART7] = std::make_unique<UART7>();
    uarts[UARTInstance::UART8] = std::make_unique<UART8>();
}

USART1* UARTManager::getUSART1() {
    return static_cast<USART1*>(uarts[UARTInstance::USART1].get());
}

USART2* UARTManager::getUSART2() {
    return static_cast<USART2*>(uarts[UARTInstance::USART2].get());
}

USART3* UARTManager::getUSART3() {
    return static_cast<USART3*>(uarts[UARTInstance::USART3].get());
}

UART4* UARTManager::getUART4() {
    return static_cast<UART4*>(uarts[UARTInstance::UART4].get());
}

UART5* UARTManager::getUART5() {
    return static_cast<UART5*>(uarts[UARTInstance::UART5].get());
}

USART6* UARTManager::getUSART6() {
    return static_cast<USART6*>(uarts[UARTInstance::USART6].get());
}

UART7* UARTManager::getUART7() {
    return static_cast<UART7*>(uarts[UARTInstance::UART7].get());
}

UART8* UARTManager::getUART8() {
    return static_cast<UART8*>(uarts[UARTInstance::UART8].get());
}

UARTPeripheral* UARTManager::getUART(UARTInstance inst) {
    return uarts[inst].get();
}

void UARTManager::initAll() {
    for (auto& uart : uarts) {
        // Default configuration
        UARTConfig config;
        config.instance = uart.first;
        uart.second->init(config);
    }
}

void UARTManager::printAllStatus() const {
    std::cout << "\n=== UART Manager Status ===" << std::endl;
    for (const auto& uart : uarts) {
        uart.second->printStatus();
    }
}

} // namespace STM32F429
