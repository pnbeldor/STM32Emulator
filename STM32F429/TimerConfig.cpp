/* --- TimerConfig.cpp --- */

/* ------------------------------------------
author: Pnbeldor
date: 3/15/2026
------------------------------------------ */

#include "TimerConfig.h"

#include <iomanip>
#include <cmath>

namespace STM32F429 {

//=============================================================================
// TimerPeripheral Implementation
//=============================================================================

TimerPeripheral::TimerPeripheral(TimerInstance inst) 
    : instance(inst),
      initialized(false),
      enabled(false),
      counter(0) {
    
    for (int i = 0; i < 4; i++) {
        channelValues[i] = 0;
        channelPolarity[i] = 0;
    }
    
    startTime = std::chrono::steady_clock::now();
    lastUpdate = startTime;
}

TimerPeripheral::~TimerPeripheral() {
    deinit();
}

uint32_t TimerPeripheral::getBaseAddress() const {
    switch(instance) {
        case TimerInstance::TIM1: return 0x40010000;
        case TimerInstance::TIM2: return 0x40000000;
        case TimerInstance::TIM3: return 0x40000400;
        case TimerInstance::TIM4: return 0x40000800;
        case TimerInstance::TIM5: return 0x40000C00;
        case TimerInstance::TIM6: return 0x40001000;
        case TimerInstance::TIM7: return 0x40001400;
        case TimerInstance::TIM8: return 0x40010400;
        case TimerInstance::TIM9: return 0x40014000;
        case TimerInstance::TIM10: return 0x40014400;
        case TimerInstance::TIM11: return 0x40014800;
        case TimerInstance::TIM12: return 0x40001800;
        case TimerInstance::TIM13: return 0x40001C00;
        case TimerInstance::TIM14: return 0x40002000;
        default: return 0;
    }
}

std::string TimerPeripheral::getInstanceName() const {
    switch(instance) {
        case TimerInstance::TIM1: return "TIM1";
        case TimerInstance::TIM2: return "TIM2";
        case TimerInstance::TIM3: return "TIM3";
        case TimerInstance::TIM4: return "TIM4";
        case TimerInstance::TIM5: return "TIM5";
        case TimerInstance::TIM6: return "TIM6";
        case TimerInstance::TIM7: return "TIM7";
        case TimerInstance::TIM8: return "TIM8";
        case TimerInstance::TIM9: return "TIM9";
        case TimerInstance::TIM10: return "TIM10";
        case TimerInstance::TIM11: return "TIM11";
        case TimerInstance::TIM12: return "TIM12";
        case TimerInstance::TIM13: return "TIM13";
        case TimerInstance::TIM14: return "TIM14";
        default: return "Unknown";
    }
}

uint32_t TimerPeripheral::getTimerClockFreq() const {
    // TIM1, TIM8-11 on APB2
    // TIM2-7, TIM12-14 on APB1
    bool isAPB2 = (instance == TimerInstance::TIM1 || 
                   instance == TimerInstance::TIM8 ||
                   instance == TimerInstance::TIM9 ||
                   instance == TimerInstance::TIM10 ||
                   instance == TimerInstance::TIM11);
    
    return config.getTimerClock(isAPB2);
}

void TimerPeripheral::updateCounter() {
    if (!enabled) return;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - lastUpdate);
    
    // Calculate how many ticks should have occurred
    uint32_t timerFreq = getTimerClockFreq() / (config.prescaler + 1);
    uint32_t ticks = static_cast<uint32_t>((elapsed.count() * timerFreq) / 1000000);
    
    if (ticks > 0) {
        lastUpdate = now;
        
        // Update counter based on mode
        if (config.counterMode == TimerCounterMode::UP) {
            counter += ticks;
            if (counter > config.autoReload) {
                statistics.overflowEvents++;
                counter %= (config.autoReload + 1);
                
                if (updateCallback) {
                    updateCallback();
                }
                
                if (overflowCallback) {
                    overflowCallback(counter);
                }
            }
        } else if (config.counterMode == TimerCounterMode::DOWN) {
            if (counter < ticks) {
                counter = config.autoReload - (ticks - counter - 1);
                statistics.overflowEvents++;
                
                if (updateCallback) {
                    updateCallback();
                }
                
                if (overflowCallback) {
                    overflowCallback(counter);
                }
            } else {
                counter -= ticks;
            }
        }
        
        // Update statistics
        if (counter > statistics.maxCounter) statistics.maxCounter = counter;
        if (counter < statistics.minCounter) statistics.minCounter = counter;
        
        // Check compare events
        for (int i = 0; i < 4; i++) {
            if (counter == channelValues[i] && compareCallback[i]) {
                compareCallback[i](static_cast<TimerChannel>(i), counter);
                statistics.compareEvents[i]++;
            }
        }
    }
}

bool TimerPeripheral::init(const TimerConfig& cfg) {
    config = cfg;
    
    // Validate based on timer type
    if (isBasicTimer(instance) && cfg.mode != TimerMode::BASIC) {
        std::cerr << "Error: Basic timer " << getInstanceName() 
                  << " only supports BASIC mode" << std::endl;
        return false;
    }
    
    if (isAdvancedTimer(instance) && cfg.mode != TimerMode::ADVANCED) {
        std::cout << "Note: Advanced timer " << getInstanceName() 
                  << " configured in non-advanced mode" << std::endl;
    }
    
    initialized = true;
    
    std::cout << "Initializing " << getInstanceName() << std::endl;
    std::cout << "  Mode: ";
    switch(cfg.mode) {
        case TimerMode::BASIC: std::cout << "BASIC"; break;
        case TimerMode::GENERAL_PURPOSE: std::cout << "GENERAL PURPOSE"; break;
        case TimerMode::ADVANCED: std::cout << "ADVANCED"; break;
        case TimerMode::PWM: std::cout << "PWM"; break;
        case TimerMode::INPUT_CAPTURE: std::cout << "INPUT CAPTURE"; break;
        case TimerMode::OUTPUT_COMPARE: std::cout << "OUTPUT COMPARE"; break;
        case TimerMode::ONE_PULSE: std::cout << "ONE PULSE"; break;
        case TimerMode::ENCODER: std::cout << "ENCODER"; break;
        case TimerMode::HALL_SENSOR: std::cout << "HALL SENSOR"; break;
    }
    std::cout << std::endl;
    
    std::cout << "  Prescaler: " << cfg.prescaler << std::endl;
    std::cout << "  Auto-reload: " << cfg.autoReload << std::endl;
    std::cout << "  Timer Freq: " << getTimerClockFreq() / (cfg.prescaler + 1) << " Hz" << std::endl;
    std::cout << "  Update Freq: " << cfg.getUpdateFrequency() << " Hz" << std::endl;
    std::cout << "  Period: " << cfg.getPeriodMs() << " ms" << std::endl;
    
    return true;
}

void TimerPeripheral::deinit() {
    disable();
    initialized = false;
    std::cout << "Deinitialized " << getInstanceName() << std::endl;
}

bool TimerPeripheral::configureChannel(TimerChannel channel, const TimerChannelConfig& chCfg) {
    int idx = static_cast<int>(channel);
    channelValues[idx] = chCfg.compareValue;
    channelPolarity[idx] = static_cast<uint32_t>(chCfg.polarity);
    
    std::cout << getInstanceName() << " Channel " << (idx + 1) << " configured" << std::endl;
    std::cout << "  Mode: " << static_cast<int>(chCfg.mode) << std::endl;
    std::cout << "  Compare Value: " << chCfg.compareValue << std::endl;
    
    return true;
}

bool TimerPeripheral::enable() {
    if (!initialized) {
        std::cerr << "Error: " << getInstanceName() << " not initialized" << std::endl;
        return false;
    }
    
    enabled = true;
    lastUpdate = std::chrono::steady_clock::now();
    
    std::cout << "Enabled " << getInstanceName() << std::endl;
    
    return true;
}

void TimerPeripheral::disable() {
    enabled = false;
    std::cout << "Disabled " << getInstanceName() << std::endl;
}

bool TimerPeripheral::start() {
    return enable();
}

void TimerPeripheral::stop() {
    disable();
}

void TimerPeripheral::reset() {
    counter = 0;
    lastUpdate = std::chrono::steady_clock::now();
    std::cout << "Reset " << getInstanceName() << std::endl;
}

void TimerPeripheral::setCounter(uint32_t value) {
    counter = value;
    lastUpdate = std::chrono::steady_clock::now();
}

void TimerPeripheral::setAutoReload(uint32_t value) {
    config.autoReload = value;
}

void TimerPeripheral::setPrescaler(uint32_t value) {
    config.prescaler = value;
}

void TimerPeripheral::setCompareValue(TimerChannel channel, uint32_t value) {
    int idx = static_cast<int>(channel);
    channelValues[idx] = value;
}

uint32_t TimerPeripheral::getCompareValue(TimerChannel channel) const {
    return channelValues[static_cast<int>(channel)];
}

uint32_t TimerPeripheral::getCaptureValue(TimerChannel channel) const {
    // In capture mode, return last captured value
    return channelValues[static_cast<int>(channel)];
}

void TimerPeripheral::forceOutput(TimerChannel channel, bool active) {
    std::cout << getInstanceName() << " Channel " << (static_cast<int>(channel) + 1)
              << " forced " << (active ? "ACTIVE" : "INACTIVE") << std::endl;
}

void TimerPeripheral::setPWMDutyCycle(TimerChannel channel, float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    
    uint32_t compare = static_cast<uint32_t>((percent / 100.0f) * config.autoReload);
    setCompareValue(channel, compare);
    
    std::cout << getInstanceName() << " Channel " << (static_cast<int>(channel) + 1)
              << " PWM duty cycle: " << percent << "%" << std::endl;
}

float TimerPeripheral::getPWMDutyCycle(TimerChannel channel) const {
    return (static_cast<float>(channelValues[static_cast<int>(channel)]) / config.autoReload) * 100.0f;
}

void TimerPeripheral::setPWMFrequency(uint32_t freqHz) {
    uint32_t timerFreq = getTimerClockFreq() / (config.prescaler + 1);
    config.autoReload = timerFreq / freqHz - 1;
    
    std::cout << getInstanceName() << " PWM frequency set to " << freqHz << " Hz" << std::endl;
}

uint32_t TimerPeripheral::getPWMFrequency() const {
    uint32_t timerFreq = getTimerClockFreq() / (config.prescaler + 1);
    return timerFreq / (config.autoReload + 1);
}

void TimerPeripheral::setEncoderMode(bool x2, bool inverted) {
    // To be overridden by derived classes
}

int32_t TimerPeripheral::getEncoderCount() const {
    return static_cast<int32_t>(counter);
}

bool TimerPeripheral::isUpdatePending() const {
    return false; // Simplified
}

bool TimerPeripheral::isCapturePending(TimerChannel channel) const {
    return false; // Simplified
}

void TimerPeripheral::onUpdate(std::function<void()> callback) {
    updateCallback = callback;
}

void TimerPeripheral::onOverflow(std::function<void(uint32_t)> callback) {
    overflowCallback = callback;
}

void TimerPeripheral::onCapture(TimerChannel channel, std::function<void(TimerChannel, uint32_t)> callback) {
    captureCallback[static_cast<int>(channel)] = callback;
}

void TimerPeripheral::onCompare(TimerChannel channel, std::function<void(TimerChannel, uint32_t)> callback) {
    compareCallback[static_cast<int>(channel)] = callback;
}

void TimerPeripheral::onTrigger(std::function<void()> callback) {
    triggerCallback = callback;
}

void TimerPeripheral::resetStatistics() {
    statistics.reset();
}

void TimerPeripheral::simulateUpdateEvent() {
    statistics.updateEvents++;
    
    if (updateCallback) {
        updateCallback();
    }
    
    std::cout << getInstanceName() << " Update event" << std::endl;
}

void TimerPeripheral::simulateCaptureEvent(TimerChannel channel, uint32_t value) {
    int idx = static_cast<int>(channel);
    channelValues[idx] = value;
    statistics.captureEvents[idx]++;
    
    if (captureCallback[idx]) {
        captureCallback[idx](channel, value);
    }
    
    std::cout << getInstanceName() << " Channel " << (idx + 1)
              << " capture: " << value << std::endl;
}

void TimerPeripheral::simulateCompareEvent(TimerChannel channel) {
    int idx = static_cast<int>(channel);
    statistics.compareEvents[idx]++;
    
    if (compareCallback[idx]) {
        compareCallback[idx](channel, channelValues[idx]);
    }
    
    std::cout << getInstanceName() << " Channel " << (idx + 1)
              << " compare match" << std::endl;
}

void TimerPeripheral::simulateOverflow() {
    statistics.overflowEvents++;
    
    if (overflowCallback) {
        overflowCallback(counter);
    }
    
    std::cout << getInstanceName() << " Overflow" << std::endl;
}

void TimerPeripheral::printConfig() const {
    std::cout << "\n=== " << getInstanceName() << " Configuration ===" << std::endl;
    std::cout << "Base Address: 0x" << std::hex << getBaseAddress() << std::dec << std::endl;
    std::cout << "Initialized: " << (initialized ? "Yes" : "No") << std::endl;
    std::cout << "Enabled: " << (enabled ? "Yes" : "No") << std::endl;
    std::cout << "Mode: " << static_cast<int>(config.mode) << std::endl;
    std::cout << "Prescaler: " << config.prescaler << std::endl;
    std::cout << "Auto-reload: " << config.autoReload << std::endl;
    std::cout << "Timer Clock: " << getTimerClockFreq() << " Hz" << std::endl;
    std::cout << "Timer Frequency: " << getTimerClockFreq() / (config.prescaler + 1) << " Hz" << std::endl;
    std::cout << "Update Frequency: " << config.getUpdateFrequency() << " Hz" << std::endl;
    std::cout << "Period: " << config.getPeriodMs() << " ms" << std::endl;
}

void TimerPeripheral::printStatus() const {
    std::cout << "\n=== " << getInstanceName() << " Status ===" << std::endl;
    std::cout << "Counter: " << counter << std::endl;
    std::cout << "Update Events: " << statistics.updateEvents << std::endl;
    std::cout << "Overflow Events: " << statistics.overflowEvents << std::endl;
    std::cout << "Max Counter: " << statistics.maxCounter << std::endl;
    std::cout << "Min Counter: " << statistics.minCounter << std::endl;
    
    for (int i = 0; i < 4; i++) {
        if (channelValues[i] != 0 || statistics.captureEvents[i] > 0) {
            std::cout << "Channel " << (i + 1) << ": Value=" << channelValues[i]
                      << ", Captures=" << statistics.captureEvents[i]
                      << ", Compares=" << statistics.compareEvents[i] << std::endl;
        }
    }
}

std::string TimerPeripheral::getInstanceString(TimerInstance inst) {
    switch(inst) {
        case TimerInstance::TIM1: return "TIM1";
        case TimerInstance::TIM2: return "TIM2";
        case TimerInstance::TIM3: return "TIM3";
        case TimerInstance::TIM4: return "TIM4";
        case TimerInstance::TIM5: return "TIM5";
        case TimerInstance::TIM6: return "TIM6";
        case TimerInstance::TIM7: return "TIM7";
        case TimerInstance::TIM8: return "TIM8";
        case TimerInstance::TIM9: return "TIM9";
        case TimerInstance::TIM10: return "TIM10";
        case TimerInstance::TIM11: return "TIM11";
        case TimerInstance::TIM12: return "TIM12";
        case TimerInstance::TIM13: return "TIM13";
        case TimerInstance::TIM14: return "TIM14";
        default: return "Unknown";
    }
}

bool TimerPeripheral::isAdvancedTimer(TimerInstance inst) {
    return (inst == TimerInstance::TIM1 || inst == TimerInstance::TIM8);
}

bool TimerPeripheral::isBasicTimer(TimerInstance inst) {
    return (inst == TimerInstance::TIM6 || inst == TimerInstance::TIM7);
}

//=============================================================================
// AdvancedTimer Implementation
//=============================================================================

AdvancedTimer::AdvancedTimer(TimerInstance inst) 
    : TimerPeripheral(inst),
      repetitionCounter(0),
      deadTime(0),
      breakInput(false),
      break2Input(false) {
    
    for (int i = 0; i < 4; i++) {
        complementaryOutputs[i] = false;
    }
}

void AdvancedTimer::setRepetitionCounter(uint32_t value) {
    repetitionCounter = value;
    std::cout << getInstanceName() << " Repetition counter set to " << value << std::endl;
}

void AdvancedTimer::setDeadTime(uint32_t ns) {
    deadTime = ns;
    std::cout << getInstanceName() << " Dead time set to " << ns << " ns" << std::endl;
}

void AdvancedTimer::enableBreak(bool enable) {
    breakInput = enable;
    std::cout << getInstanceName() << " Break " << (enable ? "enabled" : "disabled") << std::endl;
}

void AdvancedTimer::enableBreak2(bool enable) {
    break2Input = enable;
    std::cout << getInstanceName() << " Break2 " << (enable ? "enabled" : "disabled") << std::endl;
}

void AdvancedTimer::enableComplementaryOutput(TimerChannel channel, bool enable) {
    complementaryOutputs[static_cast<int>(channel)] = enable;
    std::cout << getInstanceName() << " Channel " << (static_cast<int>(channel) + 1)
              << " complementary output " << (enable ? "enabled" : "disabled") << std::endl;
}

void AdvancedTimer::setOutputPolarity(TimerChannel channel, TimerPolarity polarity, bool complementary) {
    int idx = static_cast<int>(channel);
    if (complementary) {
        std::cout << getInstanceName() << " Channel " << (idx + 1)
                  << " complementary output polarity set to " << static_cast<int>(polarity) << std::endl;
    } else {
        channelPolarity[idx] = static_cast<uint32_t>(polarity);
        std::cout << getInstanceName() << " Channel " << (idx + 1)
                  << " output polarity set to " << static_cast<int>(polarity) << std::endl;
    }
}

void AdvancedTimer::generateBreak() {
    std::cout << getInstanceName() << " Break generated" << std::endl;
    // In real hardware, this would disable outputs
}

void AdvancedTimer::clearBreak() {
    std::cout << getInstanceName() << " Break cleared" << std::endl;
}

void AdvancedTimer::printConfig() const {
    TimerPeripheral::printConfig();
    std::cout << "Repetition Counter: " << repetitionCounter << std::endl;
    std::cout << "Dead Time: " << deadTime << " ns" << std::endl;
    std::cout << "Break: " << (breakInput ? "Enabled" : "Disabled") << std::endl;
    std::cout << "Break2: " << (break2Input ? "Enabled" : "Disabled") << std::endl;
}

//=============================================================================
// BasicTimer Implementation
//=============================================================================

BasicTimer::BasicTimer(TimerInstance inst) : TimerPeripheral(inst) {}

void BasicTimer::setDutyCycle(float percent) {
    // Used for DAC trigger generation
    uint32_t compare = static_cast<uint32_t>((percent / 100.0f) * config.autoReload);
    setCompareValue(TimerChannel::CH1, compare);
    std::cout << getInstanceName() << " DAC duty cycle: " << percent << "%" << std::endl;
}

//=============================================================================
// GeneralPurposeTimer Implementation
//=============================================================================

GeneralPurposeTimer::GeneralPurposeTimer(TimerInstance inst) 
    : TimerPeripheral(inst),
      encoderMode(false),
      hallSensorMode(false),
      encoderCount(0) {}

void GeneralPurposeTimer::enableEncoderMode(bool x2, bool inverted) {
    encoderMode = true;
    encoderCount = 0;
    std::cout << getInstanceName() << " Encoder mode " << (x2 ? "x2" : "x1")
              << (inverted ? " inverted" : "") << " enabled" << std::endl;
}

void GeneralPurposeTimer::resetEncoderCount() {
    encoderCount = 0;
    counter = 0;
}

void GeneralPurposeTimer::enableHallSensorMode() {
    hallSensorMode = true;
    std::cout << getInstanceName() << " Hall sensor mode enabled" << std::endl;
}

uint8_t GeneralPurposeTimer::getHallPattern() const {
    // Simulate hall sensor pattern
    return (counter % 6) + 1;
}

void GeneralPurposeTimer::triggerOnePulse(uint32_t delay, uint32_t pulseWidth) {
    std::cout << getInstanceName() << " One-pulse triggered: delay=" << delay
              << ", width=" << pulseWidth << std::endl;
    config.onePulseMode = true;
    
    // Simulate the pulse
    std::thread([this, delay, pulseWidth]() {
        std::this_thread::sleep_for(std::chrono::microseconds(delay));
        // Pulse start
        std::this_thread::sleep_for(std::chrono::microseconds(pulseWidth));
        // Pulse end
    }).detach();
}

//=============================================================================
// TimerManager Implementation
//=============================================================================

TimerManager::TimerManager() {
    timers[TimerInstance::TIM1] = std::make_unique<AdvancedTimer>(TimerInstance::TIM1);
    timers[TimerInstance::TIM2] = std::make_unique<GeneralPurposeTimer>(TimerInstance::TIM2);
    timers[TimerInstance::TIM3] = std::make_unique<GeneralPurposeTimer>(TimerInstance::TIM3);
    timers[TimerInstance::TIM4] = std::make_unique<GeneralPurposeTimer>(TimerInstance::TIM4);
    timers[TimerInstance::TIM5] = std::make_unique<GeneralPurposeTimer>(TimerInstance::TIM5);
    timers[TimerInstance::TIM6] = std::make_unique<BasicTimer>(TimerInstance::TIM6);
    timers[TimerInstance::TIM7] = std::make_unique<BasicTimer>(TimerInstance::TIM7);
    timers[TimerInstance::TIM8] = std::make_unique<AdvancedTimer>(TimerInstance::TIM8);
    timers[TimerInstance::TIM9] = std::make_unique<GeneralPurposeTimer>(TimerInstance::TIM9);
    timers[TimerInstance::TIM10] = std::make_unique<GeneralPurposeTimer>(TimerInstance::TIM10);
    timers[TimerInstance::TIM11] = std::make_unique<GeneralPurposeTimer>(TimerInstance::TIM11);
    timers[TimerInstance::TIM12] = std::make_unique<GeneralPurposeTimer>(TimerInstance::TIM12);
    timers[TimerInstance::TIM13] = std::make_unique<GeneralPurposeTimer>(TimerInstance::TIM13);
    timers[TimerInstance::TIM14] = std::make_unique<GeneralPurposeTimer>(TimerInstance::TIM14);
}

AdvancedTimer* TimerManager::getTIM1() {
    return static_cast<AdvancedTimer*>(timers[TimerInstance::TIM1].get());
}

GeneralPurposeTimer* TimerManager::getTIM2() {
    return static_cast<GeneralPurposeTimer*>(timers[TimerInstance::TIM2].get());
}

GeneralPurposeTimer* TimerManager::getTIM3() {
    return static_cast<GeneralPurposeTimer*>(timers[TimerInstance::TIM3].get());
}

GeneralPurposeTimer* TimerManager::getTIM4() {
    return static_cast<GeneralPurposeTimer*>(timers[TimerInstance::TIM4].get());
}

GeneralPurposeTimer* TimerManager::getTIM5() {
    return static_cast<GeneralPurposeTimer*>(timers[TimerInstance::TIM5].get());
}

BasicTimer* TimerManager::getTIM6() {
    return static_cast<BasicTimer*>(timers[TimerInstance::TIM6].get());
}

BasicTimer* TimerManager::getTIM7() {
    return static_cast<BasicTimer*>(timers[TimerInstance::TIM7].get());
}

AdvancedTimer* TimerManager::getTIM8() {
    return static_cast<AdvancedTimer*>(timers[TimerInstance::TIM8].get());
}

GeneralPurposeTimer* TimerManager::getTIM9() {
    return static_cast<GeneralPurposeTimer*>(timers[TimerInstance::TIM9].get());
}

GeneralPurposeTimer* TimerManager::getTIM10() {
    return static_cast<GeneralPurposeTimer*>(timers[TimerInstance::TIM10].get());
}

GeneralPurposeTimer* TimerManager::getTIM11() {
    return static_cast<GeneralPurposeTimer*>(timers[TimerInstance::TIM11].get());
}

GeneralPurposeTimer* TimerManager::getTIM12() {
    return static_cast<GeneralPurposeTimer*>(timers[TimerInstance::TIM12].get());
}

GeneralPurposeTimer* TimerManager::getTIM13() {
    return static_cast<GeneralPurposeTimer*>(timers[TimerInstance::TIM13].get());
}

GeneralPurposeTimer* TimerManager::getTIM14() {
    return static_cast<GeneralPurposeTimer*>(timers[TimerInstance::TIM14].get());
}

TimerPeripheral* TimerManager::getTimer(TimerInstance inst) {
    return timers[inst].get();
}

void TimerManager::initAll() {
    for (auto& timer : timers) {
        TimerConfig config;
        config.instance = timer.first;
        timer.second->init(config);
    }
}

void TimerManager::startAll() {
    for (auto& timer : timers) {
        timer.second->start();
    }
}

void TimerManager::stopAll() {
    for (auto& timer : timers) {
        timer.second->stop();
    }
}

void TimerManager::printAllStatus() const {
    std::cout << "\n=== Timer Manager Status ===" << std::endl;
    for (const auto& timer : timers) {
        timer.second->printStatus();
    }
}

} // namespace STM32F429

