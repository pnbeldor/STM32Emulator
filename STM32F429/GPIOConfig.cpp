/* --- GPIOConfig.cpp --- */

/* ------------------------------------------
author: Pnbeldor
date: 3/12/2026
------------------------------------------ */

#include "GPIOConfig.h"
#include <algorithm>
#include <iomanip>

namespace STM32F429 {

//=============================================================================
// GPIOConfig Implementation
//=============================================================================

GPIOConfig::GPIOConfig() {
    // Initialize with no configurations
}

uint32_t GPIOConfig::getPortBase(GPIOPort port) const {
    switch(port) {
        case GPIOPort::PORTA: return 0x40020000;
        case GPIOPort::PORTB: return 0x40020400;
        case GPIOPort::PORTC: return 0x40020800;
        case GPIOPort::PORTD: return 0x40020C00;
        case GPIOPort::PORTE: return 0x40021000;
        case GPIOPort::PORTF: return 0x40021400;
        case GPIOPort::PORTG: return 0x40021800;
        case GPIOPort::PORTH: return 0x40021C00;
        case GPIOPort::PORTI: return 0x40022000;
        case GPIOPort::PORTJ: return 0x40022400;
        case GPIOPort::PORTK: return 0x40022800;
        default: return 0;
    }
}

std::string GPIOConfig::getPortName(GPIOPort port) const {
    switch(port) {
        case GPIOPort::PORTA: return "GPIOA";
        case GPIOPort::PORTB: return "GPIOB";
        case GPIOPort::PORTC: return "GPIOC";
        case GPIOPort::PORTD: return "GPIOD";
        case GPIOPort::PORTE: return "GPIOE";
        case GPIOPort::PORTF: return "GPIOF";
        case GPIOPort::PORTG: return "GPIOG";
        case GPIOPort::PORTH: return "GPIOH";
        case GPIOPort::PORTI: return "GPIOI";
        case GPIOPort::PORTJ: return "GPIOJ";
        case GPIOPort::PORTK: return "GPIOK";
        default: return "UNKNOWN";
    }
}

void GPIOConfig::configurePin(const NamedPin& pin, PinMode mode,
                              OutputType outputType, OutputSpeed speed,
                              PullMode pull, AlternateFunction alt) {
    PinConfig config;
    config.port = pin.port;
    config.pin = pin.pin;
    config.mode = mode;
    config.outputType = outputType;
    config.speed = speed;
    config.pull = pull;
    config.alternate = alt;
    config.isConfigured = true;
    config.name = pin.name;
    config.description = pin.description;
    
    pinConfigs[pin.name] = config;
    
    std::cout << "Configured " << pin.name << " (" << pin.description << ")" << std::endl;
    std::cout << "  Port: " << getPortName(pin.port) << ", Pin: " << (int)pin.pin << std::endl;
    std::cout << "  Mode: " << (int)mode << ", Speed: " << (int)speed << std::endl;
}

void GPIOConfig::setInput(const NamedPin& pin, PullMode pull) {
    configurePin(pin, PinMode::INPUT, OutputType::PUSH_PULL, 
                 OutputSpeed::LOW, pull, AlternateFunction::AF0);
}

void GPIOConfig::setOutput(const NamedPin& pin, OutputType outputType, OutputSpeed speed) {
    configurePin(pin, PinMode::OUTPUT, outputType, speed, 
                 PullMode::NO_PULL, AlternateFunction::AF0);
}

void GPIOConfig::setAlternate(const NamedPin& pin, AlternateFunction alt,
                              OutputType outputType, OutputSpeed speed,
                              PullMode pull) {
    configurePin(pin, PinMode::ALTERNATE, outputType, speed, pull, alt);
}

void GPIOConfig::setAnalog(const NamedPin& pin) {
    configurePin(pin, PinMode::ANALOG, OutputType::PUSH_PULL,
                 OutputSpeed::LOW, PullMode::NO_PULL, AlternateFunction::AF0);
}

void GPIOConfig::setPin(const NamedPin& pin, bool state) {
    auto it = pinConfigs.find(pin.name);
    if (it != pinConfigs.end() && it->second.isConfigured) {
        if (it->second.mode == PinMode::OUTPUT || it->second.mode == PinMode::ALTERNATE) {
            std::cout << "Setting " << pin.name << " to " << (state ? "HIGH" : "LOW") << std::endl;
        } else {
            std::cout << "Warning: " << pin.name << " is not configured as output" << std::endl;
        }
    } else {
        std::cout << "Warning: " << pin.name << " is not configured" << std::endl;
    }
}

bool GPIOConfig::getPin(const NamedPin& pin) const {
    auto it = pinConfigs.find(pin.name);
    if (it != pinConfigs.end() && it->second.isConfigured) {
        // In a real implementation, this would read the actual pin state
        return false;
    }
    std::cout << "Warning: " << pin.name << " is not configured" << std::endl;
    return false;
}

void GPIOConfig::togglePin(const NamedPin& pin) {
    auto it = pinConfigs.find(pin.name);
    if (it != pinConfigs.end() && it->second.isConfigured) {
        if (it->second.mode == PinMode::OUTPUT || it->second.mode == PinMode::ALTERNATE) {
            std::cout << "Toggling " << pin.name << std::endl;
        } else {
            std::cout << "Warning: " << pin.name << " is not configured as output" << std::endl;
        }
    } else {
        std::cout << "Warning: " << pin.name << " is not configured" << std::endl;
    }
}

void GPIOConfig::configurePins(const std::vector<NamedPin>& pins, PinMode mode,
                               OutputType outputType, OutputSpeed speed,
                               PullMode pull, AlternateFunction alt) {
    for (const auto& pin : pins) {
        configurePin(pin, mode, outputType, speed, pull, alt);
    }
}

void GPIOConfig::printConfig(const NamedPin& pin) const {
    auto it = pinConfigs.find(pin.name);
    if (it != pinConfigs.end() && it->second.isConfigured) {
        const auto& config = it->second;
        std::cout << "\n=== " << pin.name << " Configuration ===" << std::endl;
        std::cout << "Description: " << config.description << std::endl;
        std::cout << "Port: " << getPortName(config.port) << ", Pin: " << (int)config.pin << std::endl;
        std::cout << "Mode: " << (int)config.mode << std::endl;
        std::cout << "Output Type: " << (config.outputType == OutputType::PUSH_PULL ? "Push-Pull" : "Open-Drain") << std::endl;
        std::cout << "Speed: " << (int)config.speed << std::endl;
        std::cout << "Pull: " << (int)config.pull << std::endl;
        std::cout << "Alternate Function: AF" << (int)config.alternate << std::endl;
    } else {
        std::cout << pin.name << " is not configured" << std::endl;
    }
}

void GPIOConfig::printAllConfigs() const {
    if (pinConfigs.empty()) {
        std::cout << "No pins configured" << std::endl;
        return;
    }
    
    std::cout << "\n=== All Configured Pins ===" << std::endl;
    std::cout << std::left << std::setw(20) << "Pin Name" 
              << std::setw(10) << "Port" 
              << std::setw(6) << "Pin" 
              << std::setw(8) << "Mode" 
              << std::setw(10) << "Speed" << std::endl;
    std::cout << std::string(54, '-') << std::endl;
    
    for (const auto& pair : pinConfigs) {
        if (pair.second.isConfigured) {
            std::cout << std::left << std::setw(20) << pair.first
                      << std::setw(10) << getPortName(pair.second.port)
                      << std::setw(6) << (int)pair.second.pin
                      << std::setw(8) << (int)pair.second.mode
                      << std::setw(10) << (int)pair.second.speed << std::endl;
        }
    }
}

void GPIOConfig::printPortConfig(GPIOPort port) const {
    bool found = false;
    std::cout << "\n=== " << getPortName(port) << " Configured Pins ===" << std::endl;
    
    for (const auto& pair : pinConfigs) {
        if (pair.second.isConfigured && pair.second.port == port) {
            if (!found) {
                std::cout << std::left << std::setw(20) << "Pin Name" 
                          << std::setw(6) << "Pin" 
                          << std::setw(8) << "Mode" << std::endl;
                std::cout << std::string(34, '-') << std::endl;
                found = true;
            }
            std::cout << std::left << std::setw(20) << pair.first
                      << std::setw(6) << (int)pair.second.pin
                      << std::setw(8) << (int)pair.second.mode << std::endl;
        }
    }
    
    if (!found) {
        std::cout << "No pins configured on this port" << std::endl;
    }
}

bool GPIOConfig::isConfigured(const NamedPin& pin) const {
    auto it = pinConfigs.find(pin.name);
    return (it != pinConfigs.end() && it->second.isConfigured);
}

GPIOConfig::PinConfig GPIOConfig::getConfig(const NamedPin& pin) const {
    auto it = pinConfigs.find(pin.name);
    if (it != pinConfigs.end()) {
        return it->second;
    }
    return PinConfig();
}

void GPIOConfig::clearConfig(const NamedPin& pin) {
    pinConfigs.erase(pin.name);
    std::cout << "Cleared configuration for " << pin.name << std::endl;
}

void GPIOConfig::clearAllConfigs() {
    pinConfigs.clear();
    std::cout << "Cleared all pin configurations" << std::endl;
}

std::map<std::string, GPIOConfig::PinConfig> GPIOConfig::exportConfig() const {
    return pinConfigs;
}

void GPIOConfig::importConfig(const std::map<std::string, PinConfig>& config) {
    pinConfigs = config;
    std::cout << "Imported " << pinConfigs.size() << " pin configurations" << std::endl;
}

} // namespace STM32F429


/*
int main() {
    using namespace STM32F429;
    
    GPIOConfig gpio;
    
    // Configure onboard LEDs
    gpio.setOutput(PinNames::LED_GREEN, OutputType::PUSH_PULL, OutputSpeed::LOW);
    gpio.setOutput(PinNames::LED_RED, OutputType::PUSH_PULL, OutputSpeed::LOW);
    
    // Configure user button as input with pull-up
    gpio.setInput(PinNames::USER_BUTTON, PullMode::PULL_UP);
    
    // Configure UART for communication
    gpio.setAlternate(PinNames::USART2_TX, AlternateFunction::AF7);
    gpio.setAlternate(PinNames::USART2_RX, AlternateFunction::AF7);
    
    // Configure SPI for LCD
    gpio.setAlternate(PinNames::SPI2_SCK, AlternateFunction::AF5, 
                      OutputType::PUSH_PULL, OutputSpeed::VERY_HIGH);
    gpio.setAlternate(PinNames::SPI2_MOSI, AlternateFunction::AF5,
                      OutputType::PUSH_PULL, OutputSpeed::VERY_HIGH);
    gpio.setAlternate(PinNames::SPI2_MISO, AlternateFunction::AF5,
                      OutputType::PUSH_PULL, OutputSpeed::VERY_HIGH);
    
    // Configure I2C for sensors
    gpio.setAlternate(PinNames::I2C1_SCL, AlternateFunction::AF4,
                      OutputType::OPEN_DRAIN, OutputSpeed::MEDIUM, PullMode::PULL_UP);
    gpio.setAlternate(PinNames::I2C1_SDA, AlternateFunction::AF4,
                      OutputType::OPEN_DRAIN, OutputSpeed::MEDIUM, PullMode::PULL_UP);
    
    // Configure CAN
    gpio.setAlternate(PinNames::CAN1_TX, AlternateFunction::AF9);
    gpio.setAlternate(PinNames::CAN1_RX, AlternateFunction::AF9);
    
    // Configure Ethernet
    gpio.setAlternate(PinNames::ETH_MDC, AlternateFunction::AF11);
    gpio.setAlternate(PinNames::ETH_MDIO, AlternateFunction::AF11);
    
    // Configure ADC input
    gpio.setAnalog(PinNames::ADC1_IN0);
    
    // Print all configurations
    gpio.printAllConfigs();
    
    // Print specific port configuration
    gpio.printPortConfig(GPIOPort::PORTG);
    
    // Control LEDs
    gpio.setPin(PinNames::LED_GREEN, true);
    gpio.setPin(PinNames::LED_RED, false);
    gpio.togglePin(PinNames::LED_GREEN);
    
    return 0;
}
*/