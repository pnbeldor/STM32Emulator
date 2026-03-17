/* --- TimerConfig.h --- */

/* ------------------------------------------
Author: Pnbeldor
Date: 3/15/2026
------------------------------------------ */

#ifndef __TIMER_CONFIG_H__
#define __TIMER_CONFIG_H__


#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <thread>
#include <map>

namespace STM32F429 {

//=============================================================================
// Timer Instance Selection
//=============================================================================
enum class TimerInstance {
    TIM1,   // Advanced-control, APB2
    TIM2,   // General-purpose 32-bit, APB1
    TIM3,   // General-purpose 16-bit, APB1
    TIM4,   // General-purpose 16-bit, APB1
    TIM5,   // General-purpose 32-bit, APB1
    TIM6,   // Basic, APB1
    TIM7,   // Basic, APB1
    TIM8,   // Advanced-control, APB2
    TIM9,   // General-purpose 16-bit, APB2
    TIM10,  // General-purpose 16-bit, APB2
    TIM11,  // General-purpose 16-bit, APB2
    TIM12,  // General-purpose 16-bit, APB1
    TIM13,  // General-purpose 16-bit, APB1
    TIM14   // General-purpose 16-bit, APB1
};

//=============================================================================
// Timer Mode
//=============================================================================
enum class TimerMode {
    BASIC,              // Basic timer (TIM6, TIM7)
    GENERAL_PURPOSE,    // General purpose timer
    ADVANCED,           // Advanced timer (TIM1, TIM8)
    PWM,                // PWM generation
    INPUT_CAPTURE,      // Input capture mode
    OUTPUT_COMPARE,     // Output compare mode
    ONE_PULSE,          // One-pulse mode
    ENCODER,            // Encoder mode
    HALL_SENSOR         // Hall sensor mode
};

//=============================================================================
// Timer Counter Mode
//=============================================================================
enum class TimerCounterMode {
    UP,                 // Counter counts up
    DOWN,               // Counter counts down
    CENTER_ALIGNED1,    // Center-aligned, interrupt on down count
    CENTER_ALIGNED2,    // Center-aligned, interrupt on up count
    CENTER_ALIGNED3     // Center-aligned, interrupt on both
};

//=============================================================================
// Timer Clock Division
//=============================================================================
enum class TimerClockDivision {
    DIV_1 = 0,          // tDTS = tCK_INT
    DIV_2 = 1,          // tDTS = 2 x tCK_INT
    DIV_4 = 2           // tDTS = 4 x tCK_INT
};

//=============================================================================
// Timer DMA Request
//=============================================================================
enum class TimerDMARequest {
    UPDATE,             // Update event
    CAPTURE_COMPARE1,   // Capture/Compare 1
    CAPTURE_COMPARE2,   // Capture/Compare 2
    CAPTURE_COMPARE3,   // Capture/Compare 3
    CAPTURE_COMPARE4,   // Capture/Compare 4
    TRIGGER,            // Trigger
    COMMUTATION         // Commutation (advanced timers)
};

//=============================================================================
// Timer Channel
//=============================================================================
enum class TimerChannel {
    CH1,
    CH2,
    CH3,
    CH4
};

//=============================================================================
// Timer Channel Mode
//=============================================================================
enum class TimerChannelMode {
    OUTPUT_COMPARE,         // Output compare
    PWM_MODE1,              // PWM mode 1
    PWM_MODE2,              // PWM mode 2
    INPUT_CAPTURE_DIRECT,   // Input capture on TIx
    INPUT_CAPTURE_INDIRECT, // Input capture on TIx via TRC
    FORCED_OUTPUT_ACTIVE,   // Forced active output
    FORCED_OUTPUT_INACTIVE  // Forced inactive output
};

//=============================================================================
// Timer Polarity
//=============================================================================
enum class TimerPolarity {
    ACTIVE_HIGH,
    ACTIVE_LOW,
    RISING_EDGE,
    FALLING_EDGE,
    BOTH_EDGES
};

//=============================================================================
// Timer Configuration Structure
//=============================================================================
struct TimerConfig {
    TimerInstance instance;
    TimerMode mode;
    TimerCounterMode counterMode;
    TimerClockDivision clockDivision;
    uint32_t prescaler;         // 16-bit prescaler (0-65535)
    uint32_t autoReload;        // Auto-reload value
    uint32_t repetitionCounter; // Repetition counter (advanced timers only)
    bool autoReloadPreload;     // Auto-reload preload enable
    bool onePulseMode;          // One-pulse mode
    bool dmaRequests;           // Enable DMA requests
    bool updateInterrupt;       // Enable update interrupt
    uint32_t masterModeTrigger; // Master mode selection
    
    TimerConfig() : instance(TimerInstance::TIM2),
                    mode(TimerMode::GENERAL_PURPOSE),
                    counterMode(TimerCounterMode::UP),
                    clockDivision(TimerClockDivision::DIV_1),
                    prescaler(0),
                    autoReload(0xFFFF),
                    repetitionCounter(0),
                    autoReloadPreload(false),
                    onePulseMode(false),
                    dmaRequests(false),
                    updateInterrupt(false),
                    masterModeTrigger(0) {}
    
    // Get timer clock frequency in Hz
    uint32_t getTimerClock(bool isAPB2 = false) const {
        // APB1 timers (TIM2-7, TIM12-14) max 90/180 MHz depending on config
        // APB2 timers (TIM1, TIM8-11) max 180 MHz
        if (isAPB2) {
            return 180000000;  // 180 MHz max for APB2 timers
        } else {
            return 90000000;   // 90 MHz max for APB1 timers
        }
    }
    
    // Calculate timer frequency in Hz
    uint32_t getTimerFrequency(bool isAPB2 = false) const {
        uint32_t timerClk = getTimerClock(isAPB2);
        return timerClk / (prescaler + 1);
    }
    
    // Calculate update event frequency in Hz
    uint32_t getUpdateFrequency(bool isAPB2 = false) const {
        return getTimerFrequency(isAPB2) / (autoReload + 1);
    }
    
    // Calculate period in seconds
    double getPeriodSec(bool isAPB2 = false) const {
        return 1.0 / getUpdateFrequency(isAPB2);
    }
    
    // Calculate period in milliseconds
    double getPeriodMs(bool isAPB2 = false) const {
        return getPeriodSec(isAPB2) * 1000.0;
    }
    
    // Calculate period in microseconds
    double getPeriodUs(bool isAPB2 = false) const {
        return getPeriodSec(isAPB2) * 1000000.0;
    }
};

//=============================================================================
// Timer Channel Configuration
//=============================================================================
struct TimerChannelConfig {
    TimerChannel channel;
    TimerChannelMode mode;
    TimerPolarity polarity;
    uint32_t compareValue;      // Capture/compare value
    uint32_t pulseWidth;        // Pulse width for PWM mode
    bool preloadEnable;         // Preload enable
    bool fastEnable;            // Fast enable
    bool dmaRequest;            // DMA request enable
    
    TimerChannelConfig() : channel(TimerChannel::CH1),
                           mode(TimerChannelMode::OUTPUT_COMPARE),
                           polarity(TimerPolarity::ACTIVE_HIGH),
                           compareValue(0),
                           pulseWidth(0),
                           preloadEnable(false),
                           fastEnable(false),
                           dmaRequest(false) {}
};

//=============================================================================
// Timer Statistics
//=============================================================================
struct TimerStatistics {
    uint64_t updateEvents;
    uint64_t captureEvents[4];
    uint64_t compareEvents[4];
    uint64_t overflowEvents;
    uint32_t maxCounter;
    uint32_t minCounter;
    
    TimerStatistics() : updateEvents(0), overflowEvents(0), maxCounter(0), minCounter(0xFFFFFFFF) {
        for (int i = 0; i < 4; i++) {
            captureEvents[i] = 0;
            compareEvents[i] = 0;
        }
    }
    
    void reset() {
        updateEvents = 0;
        overflowEvents = 0;
        maxCounter = 0;
        minCounter = 0xFFFFFFFF;
        for (int i = 0; i < 4; i++) {
            captureEvents[i] = 0;
            compareEvents[i] = 0;
        }
    }
};

//=============================================================================
// Timer Peripheral Base Class
//=============================================================================
class TimerPeripheral {
protected:
    TimerInstance instance;
    TimerConfig config;
    bool initialized;
    bool enabled;
    
    uint32_t counter;           // Current counter value
    uint32_t channelValues[4];   // Channel capture/compare values
    uint32_t channelPolarity[4];
    
    // Statistics
    TimerStatistics statistics;
    
    // Callbacks
    std::function<void()> updateCallback;
    std::function<void(uint32_t)> overflowCallback;
    std::function<void(TimerChannel, uint32_t)> captureCallback[4];
    std::function<void(TimerChannel, uint32_t)> compareCallback[4];
    std::function<void()> triggerCallback;
    std::function<void()> commutationCallback;  // Advanced timers only
    
    // Timing simulation
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point lastUpdate;
    
    // Helper methods
    uint32_t getBaseAddress() const;
    std::string getInstanceName() const;
    uint32_t getTimerClockFreq() const;
    void updateCounter();
    
public:
    TimerPeripheral(TimerInstance inst);
    virtual ~TimerPeripheral();
    
    // Configuration
    virtual bool init(const TimerConfig& cfg);
    virtual void deinit();
    virtual bool configureChannel(TimerChannel channel, const TimerChannelConfig& chCfg);
    
    // Control
    virtual bool enable();
    virtual void disable();
    virtual bool start();
    virtual void stop();
    virtual void reset();
    
    // Counter management
    virtual void setCounter(uint32_t value);
    virtual uint32_t getCounter() const { return counter; }
    virtual uint32_t getAutoReload() const { return config.autoReload; }
    virtual void setAutoReload(uint32_t value);
    virtual uint32_t getPrescaler() const { return config.prescaler; }
    virtual void setPrescaler(uint32_t value);
    
    // Channel management
    virtual void setCompareValue(TimerChannel channel, uint32_t value);
    virtual uint32_t getCompareValue(TimerChannel channel) const;
    virtual uint32_t getCaptureValue(TimerChannel channel) const;
    virtual void forceOutput(TimerChannel channel, bool active);
    
    // PWM specific
    virtual void setPWMDutyCycle(TimerChannel channel, float percent);
    virtual float getPWMDutyCycle(TimerChannel channel) const;
    virtual void setPWMFrequency(uint32_t freqHz);
    virtual uint32_t getPWMFrequency() const;
    
    // Encoder mode
    virtual void setEncoderMode(bool x2, bool inverted = false);
    virtual int32_t getEncoderCount() const;
    
    // Status
    virtual bool isEnabled() const { return enabled; }
    virtual bool isUpdatePending() const;
    virtual bool isCapturePending(TimerChannel channel) const;
    
    // Callback registration
    void onUpdate(std::function<void()> callback);
    void onOverflow(std::function<void(uint32_t)> callback);
    void onCapture(TimerChannel channel, std::function<void(TimerChannel, uint32_t)> callback);
    void onCompare(TimerChannel channel, std::function<void(TimerChannel, uint32_t)> callback);
    void onTrigger(std::function<void()> callback);
    
    // Statistics
    TimerStatistics getStatistics() const { return statistics; }
    void resetStatistics();
    
    // Simulation functions
    virtual void simulateUpdateEvent();
    virtual void simulateCaptureEvent(TimerChannel channel, uint32_t value);
    virtual void simulateCompareEvent(TimerChannel channel);
    virtual void simulateOverflow();
    
    // Print configuration
    virtual void printConfig() const;
    virtual void printStatus() const;
    
    // Static utility functions
    static std::string getInstanceString(TimerInstance inst);
    static bool isAdvancedTimer(TimerInstance inst);
    static bool isBasicTimer(TimerInstance inst);
    static uint32_t getMaxPrescaler() { return 65535; }
    static uint32_t getMaxAutoReload16() { return 65535; }
    static uint32_t getMaxAutoReload32() { return 0xFFFFFFFF; }
};

//=============================================================================
// Advanced Timer (TIM1, TIM8)
//=============================================================================
class AdvancedTimer : public TimerPeripheral {
private:
    uint32_t repetitionCounter;
    uint32_t deadTime;          // Dead time in ns
    bool breakInput;
    bool break2Input;
    bool complementaryOutputs[4];
    
public:
    AdvancedTimer(TimerInstance inst);
    
    // Advanced timer specific features
    void setRepetitionCounter(uint32_t value);
    void setDeadTime(uint32_t ns);
    void enableBreak(bool enable);
    void enableBreak2(bool enable);
    void enableComplementaryOutput(TimerChannel channel, bool enable);
    void setOutputPolarity(TimerChannel channel, TimerPolarity polarity, bool complementary = false);
    void generateBreak();
    void clearBreak();
    
    virtual void printConfig() const override;
};

//=============================================================================
// Basic Timer (TIM6, TIM7)
//=============================================================================
class BasicTimer : public TimerPeripheral {
public:
    BasicTimer(TimerInstance inst);
    
    // Basic timer specific features
    void setDutyCycle(float percent);  // For DAC trigger
};

//=============================================================================
// General Purpose Timer (TIM2-5, TIM9-14)
//=============================================================================
class GeneralPurposeTimer : public TimerPeripheral {
private:
    bool encoderMode;
    bool hallSensorMode;
    int32_t encoderCount;
    
public:
    GeneralPurposeTimer(TimerInstance inst);
    
    // Encoder mode
    void enableEncoderMode(bool x2, bool inverted = false);
    int32_t getEncoderCount() const { return encoderCount; }
    void resetEncoderCount();
    
    // Hall sensor mode
    void enableHallSensorMode();
    uint8_t getHallPattern() const;
    
    // One-pulse mode
    void triggerOnePulse(uint32_t delay, uint32_t pulseWidth);
};

//=============================================================================
// Timer Manager Class
//=============================================================================
class TimerManager {
private:
    std::map<TimerInstance, std::unique_ptr<TimerPeripheral>> timers;
    
public:
    TimerManager();
    
    // Get specific timer instances
    AdvancedTimer* getTIM1();
    GeneralPurposeTimer* getTIM2();
    GeneralPurposeTimer* getTIM3();
    GeneralPurposeTimer* getTIM4();
    GeneralPurposeTimer* getTIM5();
    BasicTimer* getTIM6();
    BasicTimer* getTIM7();
    AdvancedTimer* getTIM8();
    GeneralPurposeTimer* getTIM9();
    GeneralPurposeTimer* getTIM10();
    GeneralPurposeTimer* getTIM11();
    GeneralPurposeTimer* getTIM12();
    GeneralPurposeTimer* getTIM13();
    GeneralPurposeTimer* getTIM14();
    
    // Get by instance enum
    TimerPeripheral* getTimer(TimerInstance inst);
    
    // Initialize all timers
    void initAll();
    
    // Start all timers
    void startAll();
    
    // Stop all timers
    void stopAll();
    
    // Print status of all timers
    void printAllStatus() const;
};

} // namespace STM32F429

#endif // __TIMER_CONFIG_H__

