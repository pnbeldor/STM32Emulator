#include "ClockConfig.h"
#include "DMAConfig.h"
#include "I2CConfig.h"
#include "GPIOConfig.h"
#include "SPIConfig.h"
#include "CANConfig.h"
#include "ClockConfig.h"
#include "UARTConfig.h"
#include "TimerConfig.h"

#include <thread>
#include <cstdint>

int main() {
    using namespace STM32F429;
    
    //=========================================================================
    // Clock Configuration
    //=========================================================================
    ClockConfig clock;
    
    // Set external crystal frequency
    clock.setHSECrystal(8000000);  // 8 MHz crystal
    
    // Configure PLL for 168 MHz operation
    PLLConfig pll = ClockConfig::get168MHzConfig(8000000);
    clock.usePLL(pll);
    
    // Set prescalers
    clock.setAHBPrescaler(AHBPrescaler::DIV_1);      // HCLK = 168 MHz
    clock.setAPB1Prescaler(APBPrescaler::DIV_4);     // PCLK1 = 42 MHz
    clock.setAPB2Prescaler(APBPrescaler::DIV_2);     // PCLK2 = 84 MHz
    
    // Enable peripheral clocks
    clock.enablePeripheralClock("SPI2");
    clock.enablePeripheralClock("I2C1");
    clock.enablePeripheralClock("DMA1");
    clock.enablePeripheralClock("DMA2");
    clock.enablePeripheralClock("CAN1");
    clock.enablePeripheralClock("CAN2");
    
    clock.printConfig();

    //=========================================================================
    // UART/USART Configuration
    //=========================================================================
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "UART/USART TESTS" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    UARTManager uartManager;
    
    // Configure USART2 (console)
    UARTConfig uart2Config;
    uart2Config.instance = UARTInstance::USART2;
    uart2Config.baudRate = UARTBaudRate::BAUD_115200;
    uart2Config.dataBits = UARTDataBits::BITS_8;
    uart2Config.stopBits = UARTStopBits::STOP_1;
    
    auto usart2 = uartManager.getUSART2();
    usart2->init(uart2Config);
    usart2->enable();
    
    // Configure USART1 (high-speed)
    UARTConfig uart1Config;
    uart1Config.instance = UARTInstance::USART1;
    uart1Config.baudRate = UARTBaudRate::BAUD_921600;
    uart1Config.enableDMA = true;
    
    auto usart1 = uartManager.getUSART1();
    usart1->init(uart1Config);
    usart1->enableDMA(4, 5);  // DMA2 Stream 4 for TX, Stream 5 for RX
    usart1->enable();
    
    // Set up callbacks
    usart2->onDataReceived([](uint8_t data) {
        std::cout << "USART2 Received: " << data << std::endl;
    });
    
    // Test UART communication
    std::cout << "\n--- UART Transmit Test ---" << std::endl;
    usart2->transmit("Hello STM32!\r\n");
    usart1->transmit("High-speed message\r\n");
    
    // Simulate received data
    std::cout << "\n--- UART Receive Simulation ---" << std::endl;
    usart2->simulateReceive("User input\r\n");
    usart1->simulateReceive(0x55);
    
    usart2->printStatus();
    
    //=========================================================================
    // Timer Configuration
    //=========================================================================
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "TIMER TESTS" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    TimerManager timerManager;
    
    // Configure TIM2 as general purpose timer
    TimerConfig tim2Config;
    tim2Config.instance = TimerInstance::TIM2;
    tim2Config.prescaler = 8399;  // 168 MHz / 8400 = 20 kHz
    tim2Config.autoReload = 19999; // 20 kHz / 20000 = 1 Hz
    tim2Config.updateInterrupt = true;
    
    auto tim2 = timerManager.getTIM2();
    tim2->init(tim2Config);
    
    tim2->onUpdate([]() {
        std::cout << "TIM2 Update (1 second elapsed)" << std::endl;
    });
    
    // Configure TIM3 in PWM mode
    TimerConfig tim3Config;
    tim3Config.instance = TimerInstance::TIM3;
    tim3Config.mode = TimerMode::PWM;
    tim3Config.prescaler = 0;
    tim3Config.autoReload = 999;  // 84 MHz / 1000 = 84 kHz
    
    auto tim3 = timerManager.getTIM3();
    tim3->init(tim3Config);
    
    TimerChannelConfig pwmConfig;
    pwmConfig.mode = TimerChannelMode::PWM_MODE1;
    pwmConfig.polarity = TimerPolarity::ACTIVE_HIGH;
    pwmConfig.compareValue = 250;  // 25% duty cycle
    
    tim3->configureChannel(TimerChannel::CH1, pwmConfig);
    
    // Configure TIM4 in input capture mode
    TimerConfig tim4Config;
    tim4Config.instance = TimerInstance::TIM4;
    tim4Config.mode = TimerMode::INPUT_CAPTURE;
    
    auto tim4 = timerManager.getTIM4();
    tim4->init(tim4Config);
    
    tim4->onCapture(TimerChannel::CH1, [](TimerChannel ch, uint32_t value) {
        std::cout << "TIM4 Capture: " << value << std::endl;
    });
    
    // Configure TIM1 as advanced timer with dead time
    TimerConfig tim1Config;
    tim1Config.instance = TimerInstance::TIM1;
    tim1Config.mode = TimerMode::ADVANCED;
    tim1Config.prescaler = 0;
    tim1Config.autoReload = 999;
    
    auto tim1 = timerManager.getTIM1();
    tim1->init(tim1Config);
    tim1->setDeadTime(100);  // 100 ns dead time
    tim1->enableComplementaryOutput(TimerChannel::CH1, true);
    
    // Start timers
    std::cout << "\n--- Starting Timers ---" << std::endl;
    tim2->start();
    tim3->start();
    tim4->start();
    tim1->start();
    
    // Test PWM adjustment
    std::cout << "\n--- PWM Test ---" << std::endl;
    tim3->setPWMDutyCycle(TimerChannel::CH1, 50.0f);  // 50% duty cycle
    std::cout << "PWM Frequency: " << tim3->getPWMFrequency() << " Hz" << std::endl;
    
    // Test encoder mode
    std::cout << "\n--- Encoder Mode Test ---" << std::endl;
    tim2->setEncoderMode(true);
    
    // Test one-pulse mode
    std::cout << "\n--- One-Pulse Mode Test ---" << std::endl;
    tim2->triggerOnePulse(1000, 500);  // 1ms delay, 500µs pulse
    
    // Simulate timer events
    std::cout << "\n--- Timer Event Simulation ---" << std::endl;
    tim2->simulateUpdateEvent();
    tim4->simulateCaptureEvent(TimerChannel::CH1, 1234);
    tim3->simulateCompareEvent(TimerChannel::CH1);
    tim2->simulateOverflow();
    
    // Print final status
    timerManager.printAllStatus();
    
    // Let timers run for a bit
    std::this_thread::sleep_for(std::chrono::seconds(2));

    
    //=========================================================================
    // SPI Configuration
    //=========================================================================
    SPIPeripheral spi(SPIInstance::SPI2);
    
    // Set clock frequencies
    spi.setClockFrequencies(clock.getPCLK1(), clock.getPCLK2());
    
    // Configure SPI
    SPIConfig spiConfig;
    spiConfig.mode = SPIMode::MASTER;
    spiConfig.baudRate = SPIBaudRatePrescaler::DIV_8;  // 42 MHz / 8 = 5.25 MHz
    spiConfig.clockPolarity = SPIClockPolarity::LOW;
    spiConfig.clockPhase = SPIClockPhase::FIRST_EDGE;
    spiConfig.dataSize = SPIDataSize::BITS_8;
    
    spi.init(spiConfig);
    spi.enable();
    
    // LCD operations
    spi.lcdWriteCommand(0x2C);  // Memory write
    spi.lcdWriteData(0xAA);
    spi.lcdFillRect(0, 0, 100, 100, 0xFFFF);  // White rectangle
    
    spi.printConfig();
    
    //=========================================================================
    // I2C Configuration
    //=========================================================================
    I2CPeripheral i2c(I2CInstance::I2C1);
    
    i2c.setClockFrequency(clock.getPCLK1());
    
    I2CConfig i2cConfig;
    i2cConfig.mode = I2CMode::FAST;  // 400 kHz
    i2cConfig.ownAddress = 0x42;
    
    i2c.init(i2cConfig);
    i2c.enable();
    
    // Simulate I2C devices
    i2c.simulateTemperatureSensor(0x48);  // LM75 at address 0x48
    i2c.simulateEEPROM(0x50, 256);        // 24LCxx at address 0x50
    
    // Read temperature
    float temp;
    if (i2c.readTemperature(0x48, temp)) {
        std::cout << "Temperature: " << temp << "°C" << std::endl;
    }
    
    // Write to EEPROM
    i2c.writeEEPROM(0x50, 0x0010, 0xAB);
    
    // Read from EEPROM
    uint8_t data;
    i2c.readEEPROM(0x50, 0x0010, data);
    std::cout << "EEPROM[0x0010] = 0x" << std::hex << (int)data << std::dec << std::endl;
    
    i2c.printConfig();
    
    //=========================================================================
    // DMA Configuration
    //=========================================================================
    DMAPeripheral dma(DMAInstance::DMA1);
    dma.init();
    dma.enable();
    
    // Configure SPI2 TX using DMA
    auto spi2Tx = DMAPeripheral::getSPI2_TX();
    dma.configureForSPI_TX(spi2Tx.stream, spi2Tx.channel);
    
    // Configure I2C1 RX using DMA
    auto i2c1Rx = DMAPeripheral::getI2C1_RX();
    dma.configureForI2C_RX(i2c1Rx.stream, i2c1Rx.channel);
    
    // Set up memory-to-memory transfer
    uint32_t srcBuffer[100];
    uint32_t dstBuffer[100];
    
    // Initialize source buffer
    for (int i = 0; i < 100; i++) {
        srcBuffer[i] = i;
    }
    
    dma.setStreamPeripheralAddress(DMAStream::STREAM0, *srcBuffer);
    dma.setStreamMemoryAddress(DMAStream::STREAM0, *dstBuffer);
    dma.setStreamDataLength(DMAStream::STREAM0, sizeof(srcBuffer));
    
    dma.onTransferComplete(DMAStream::STREAM0, []() {
        std::cout << "DMA transfer complete!" << std::endl;
    });
    
    dma.memoryToMemoryTransfer(DMAStream::STREAM0, 
                               *srcBuffer, 
                               *dstBuffer, 
                               sizeof(srcBuffer));
    
    dma.printConfig();

        //=========================================================================
    // CAN1 Configuration
    //=========================================================================
    CANConfig can1Config;
    can1Config.instance = CANInstance::CAN1;
    can1Config.baudRate = CANBaudRate::BAUD_500K;
    can1Config.mode = CANMode::NORMAL;
    can1Config.pclk1 = clock.getPCLK1();
    
    CANPeripheral can1(CANInstance::CAN1);
    can1.init(can1Config);
    
    // Configure filters
    can1.setFilter(0, 0x123, 0x7FF, CANFilterFIFO::FIFO0);  // Accept ID 0x123
    can1.setFilter(1, 0x456, 0x7FF, CANFilterFIFO::FIFO1);  // Accept ID 0x456
    
    can1.enable();
    
    //=========================================================================
    // CAN2 Configuration
    //=========================================================================
    CANConfig can2Config;
    can2Config.instance = CANInstance::CAN2;
    can2Config.baudRate = CANBaudRate::BAUD_500K;
    can2Config.mode = CANMode::NORMAL;
    can2Config.pclk1 = clock.getPCLK1();
    
    CANPeripheral can2(CANInstance::CAN2);
    can2.init(can2Config);
    
    // Configure filters
    can2.setFilter(0, 0x789, 0x7FF, CANFilterFIFO::FIFO0);  // Accept ID 0x789
    
    can2.enable();
    
    //=========================================================================
    // Set up callbacks
    //=========================================================================
    can1.onReceive([](const CANMessage& msg) {
        std::cout << "CAN1 Received: " << msg.toString() << std::endl;
    });
    
    can2.onReceive([](const CANMessage& msg) {
        std::cout << "CAN2 Received: " << msg.toString() << std::endl;
    });
    
    can1.onError([](uint8_t code, uint8_t count) {
        std::cout << "CAN1 Error: Code=" << (int)code << " Count=" << (int)count << std::endl;
    });
    
    //=========================================================================
    // Test CAN communication
    //=========================================================================
    
    // Test 1: Send message from CAN1 to CAN2
    std::cout << "\n=== Test 1: CAN1 -> CAN2 ===" << std::endl;
    uint8_t data1[] = {0x11, 0x22, 0x33, 0x44};
    CANMessage msg1(0x123, false, 4, data1);
    
    if (can1.transmit(msg1)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Test 2: Send message from CAN2 to CAN1
    std::cout << "\n=== Test 2: CAN2 -> CAN1 ===" << std::endl;
    uint8_t data2[] = {0x55, 0x66, 0x77, 0x88};
    CANMessage msg2(0x456, false, 4, data2);
    
    if (can2.transmit(msg2)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Test 3: Extended ID message
    std::cout << "\n=== Test 3: Extended ID ===" << std::endl;
    uint8_t data3[] = {0xAA, 0xBB, 0xCC, 0xDD};
    CANMessage msg3(0x1FFFFFFF, true, 4, data3);
    
    // Configure filter for extended ID
    can1.setFilter(2, 0x1FFFFFFF, 0x1FFFFFFF, CANFilterFIFO::FIFO0);
    
    if (can1.transmit(msg3)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Test 4: Remote frame
    std::cout << "\n=== Test 4: Remote Frame ===" << std::endl;
    CANMessage remoteMsg(0x789, false, 2);  // Remote frame requesting 2 bytes
    
    if (can2.transmit(remoteMsg)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Test 5: Multiple messages
    std::cout << "\n=== Test 5: Multiple Messages ===" << std::endl;
    std::vector<CANMessage> messages;
    
    for (int i = 0; i < 5; i++) {
        uint8_t data[] = {static_cast<uint8_t>(i), static_cast<uint8_t>(i+1), 
                          static_cast<uint8_t>(i+2), static_cast<uint8_t>(i+3)};
        messages.emplace_back(0x200 + i, false, 4, data);
    }
    
    can1.transmit(messages);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Test 6: Loopback mode
    std::cout << "\n=== Test 6: Loopback Mode ===" << std::endl;
    can1.runLoopbackTest();
    
    // Test 7: Error simulation
    std::cout << "\n=== Test 7: Error Simulation ===" << std::endl;
    can1.simulateError();
    
    //=========================================================================
    // Print statistics and status
    //=========================================================================
    can1.printStatistics();
    can1.printStatus();
    
    can2.printStatistics();
    can2.printStatus();
    
    //=========================================================================
    // Test bus off condition
    //=========================================================================
    std::cout << "\n=== Test 8: Bus Off Simulation ===" << std::endl;
    can1.simulateBusOff();
    can1.printStatus();
    
    return 0;
}
