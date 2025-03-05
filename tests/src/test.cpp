#include <gtest/gtest.h>
#include <stdint.h>
#include <stdio.h>
#include <memory>
#include <math.h>
#include <numeric>

#include "UserInput.h"
#include "FfbEngine.h"
#include "FfbReportHandler.h"
#include "HIDReportType.h"
#include "helpers/hidTypesExt.hpp"

#define USB_RAD_270 (USB_MAX_PHASE - (M_PI_2 * USB_NORMALIZE_RAD))
#define USB_RAD_45 (M_PI_4 * USB_NORMALIZE_RAD)

#define ZERO_TRIGGER_REPEAT_INTERVAL 0
#define ZERO_SAMPLE_INTERVAL 0
#define ZERO_START_DELAY 0

static uint64_t current_time = 0;

static uint64_t GetFakeTime()
{
    return current_time;
}
static void TickFakeTime(unsigned int time = 1)
{
    current_time += time;
}
static void SetFakeTime(unsigned int time)
{
    current_time = time;
}
static void ResetFakeTime()
{
    current_time = 0;
}
class HidAbstractor : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ffh = std::make_unique<FfbReportHandler>(GetFakeTime);
        ffe = std::make_unique<FfbEngine>(*ffh, ui, GetFakeTime);
    }

    UserInput ui;
    std::unique_ptr<FfbReportHandler> ffh;
    std::unique_ptr<FfbEngine> ffe;

    template <typename... Args>
    int CreateEffect(Args... args)
    {
        ffh->FfbOnCreateNewEffect((USB_FFBReport_CreateNewEffect_Feature_Data_t *)0);
        int effectBlock = ffh->FfbOnPIDBlockLoad()[1];

        auto effect = Reportfactory<SetEffect_Ext>(effectBlock, args...);
        ffh->FfbOnUsbData((uint8_t *)effect.get(), sizeof(SetEffect_Ext));

        return effectBlock;
    }

    template <typename T, typename... Args>
    std::unique_ptr<T> Reportfactory(Args... args)
    {
        return std::make_unique<T>(args...);
    }

    template <typename T, typename... Args>
    void SetReport(Args... args)
    {
        auto report = Reportfactory<T>(args...);
        ffh->FfbOnUsbData((uint8_t *)report.get(), sizeof(T));
    }

    void UpdatePosition(std::array<int32_t, NUM_AXES> &&list)
    {
        ui.UpdatePosition(&list[0]);
    }

    void UpdateMetrics(std::array<std::array<int32_t, NUM_AXES>, UserInput::metricsCount> &&list)
    {
        ui.UpdateMetrics(&list[UserInput::position][0], &list[UserInput::speed][0], &list[UserInput::acceleration][0]);
    }
};
class HidAbstractorParametrized : public HidAbstractor,
                                  public ::testing::WithParamInterface<int>
{
};

TEST_F(HidAbstractor, TestAdministration)
{
    int assignedCount = 0;
    for (;;)
    {
        int effectBlock = CreateEffect(
            USB_EFFECT_CONSTANT,
            1,
            ZERO_TRIGGER_REPEAT_INTERVAL,
            ZERO_SAMPLE_INTERVAL,
            USB_MAX_GAIN,
            USB_NO_TRIGGER_BUTTON,
            X_AXIS_ENABLE,
            0,
            0,
            ZERO_START_DELAY);

        SetReport<SetConstantForce_Ext>(effectBlock, 1);
        SetReport<EffectOperation_Ext>(effectBlock, 1);
        if (effectBlock == 0)
        {
            break;
        }
        ++assignedCount;
    }
    EXPECT_EQ(assignedCount, MAX_EFFECTS);

    int forces[2] = {0};

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], MAX_EFFECTS);

    {
        auto ffbState = (USB_FFBReport_PIDBlockLoad_Feature_Data_t *)ffh->FfbOnPIDBlockLoad();
        EXPECT_EQ(ffbState->ramPoolAvailable, 0);
    }
    {
        SetReport<BlockFree_Ext>(MAX_EFFECTS);
        ffe->ForceCalculator(forces);
        EXPECT_EQ(forces[0], MAX_EFFECTS - 1);

        int effectBlock = CreateEffect(
            USB_EFFECT_CONSTANT,
            1,
            ZERO_TRIGGER_REPEAT_INTERVAL,
            ZERO_SAMPLE_INTERVAL,
            USB_MAX_GAIN,
            USB_NO_TRIGGER_BUTTON,
            X_AXIS_ENABLE,
            0,
            0,
            ZERO_START_DELAY);

        SetReport<SetConstantForce_Ext>(effectBlock, 1);
        SetReport<EffectOperation_Ext>(effectBlock, 1);
        ffe->ForceCalculator(forces);
        EXPECT_EQ(forces[0], MAX_EFFECTS);
    }
    {
        SetReport<EffectOperation_Ext>(MAX_EFFECTS / 2, 2);
        ffe->ForceCalculator(forces);
        EXPECT_EQ(forces[0], 1);
    }
    {
        SetReport<EffectOperation_Ext>(MAX_EFFECTS / 2, 3);
        ffe->ForceCalculator(forces);
        EXPECT_EQ(forces[0], 0);
    }
    {
        SetReport<BlockFree_Ext>();

        auto ffbState = (USB_FFBReport_PIDBlockLoad_Feature_Data_t *)ffh->FfbOnPIDBlockLoad();
        EXPECT_EQ(ffbState->ramPoolAvailable, MEMORY_SIZE);

        ffe->ForceCalculator(forces);
        EXPECT_EQ(forces[0], 0);
    }
}

TEST_F(HidAbstractor, TestDeviceGain)
{
    ResetFakeTime();

    int effectBlock = CreateEffect(
        USB_EFFECT_CONSTANT,
        1,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetConstantForce_Ext>(effectBlock, 100);
    SetReport<EffectOperation_Ext>(effectBlock, 1);
    int forces[2] = {0};

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 100);

    SetReport<DeviceGain_Ext>(128);
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 50);
}

TEST_F(HidAbstractor, TestDelay)
{
    ResetFakeTime();

    int effectBlock = CreateEffect(
        USB_EFFECT_CONSTANT,
        1,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        1);

    SetReport<SetConstantForce_Ext>(effectBlock, 100);
    SetReport<EffectOperation_Ext>(effectBlock, 1);
    int forces[2] = {0};

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 100);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
}

TEST_F(HidAbstractor, TestPaused)
{
    ResetFakeTime();

    int effectBlock = CreateEffect(
        USB_EFFECT_CONSTANT,
        2,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetConstantForce_Ext>(effectBlock, 100);
    SetReport<EffectOperation_Ext>(effectBlock, 1);
    int forces[2] = {0};

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 100);

    SetReport<DeviceControl_Ext>(5);
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);

    TickFakeTime();
    SetReport<DeviceControl_Ext>(6);
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 100);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 100);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
}

TEST_P(HidAbstractorParametrized, TestAllConditionEffects)
{
    int conditionType = GetParam();
    int effectBlock = CreateEffect(
        conditionType,
        USB_DURATION_INFINITE,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetCondition_Ext>(effectBlock, 0, USB_AXIS_MAX_ABSOLUTE / 4, USB_MAX_GAIN, USB_MAX_GAIN, USB_MAX_GAIN, USB_MAX_GAIN, USB_AXIS_MAX_ABSOLUTE / 4);
    SetReport<EffectOperation_Ext>(effectBlock, 1);

    int forces[2] = {0};

    UpdateMetrics({USB_AXIS_MAX_ABSOLUTE, USB_AXIS_MAX_ABSOLUTE, USB_AXIS_MAX_ABSOLUTE, USB_AXIS_MAX_ABSOLUTE, USB_AXIS_MAX_ABSOLUTE, USB_AXIS_MAX_ABSOLUTE});
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], -127);
}
INSTANTIATE_TEST_CASE_P(
    TestAllConditionEffects,
    HidAbstractorParametrized,
    ::testing::Values(
        USB_EFFECT_SPRING, USB_EFFECT_DAMPER, USB_EFFECT_INERTIA, USB_EFFECT_FRICTION));

TEST_F(HidAbstractor, TestConstantTriggerButton)
{
    ResetFakeTime();

    int effectBlock = CreateEffect(
        USB_EFFECT_CONSTANT,
        1,
        2,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        1,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetConstantForce_Ext>(effectBlock, USB_MAX_MAGNITUDE);
    SetReport<EffectOperation_Ext>(effectBlock, 1);
    int forces[2] = {0};

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);

    ui.UpdateButtons(1);
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], USB_MAX_MAGNITUDE);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);

    TickFakeTime(2);
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], USB_MAX_MAGNITUDE);

    ui.UpdateButtons(0);
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
}

TEST_F(HidAbstractor, TestConditionDirectionOffset)
{
    int effectBlock = CreateEffect(
        USB_EFFECT_SPRING,
        USB_DURATION_INFINITE,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE | Y_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetCondition_Ext>(effectBlock, 0, USB_AXIS_MAX_ABSOLUTE / 4, USB_MAX_GAIN, USB_MAX_GAIN, USB_MAX_GAIN, USB_MAX_GAIN, USB_AXIS_MAX_ABSOLUTE / 4);
    SetReport<SetCondition_Ext>(effectBlock, 1, USB_AXIS_MAX_ABSOLUTE / 4, USB_MAX_GAIN, USB_MAX_GAIN, USB_MAX_GAIN, USB_MAX_GAIN, USB_AXIS_MAX_ABSOLUTE / 4);
    SetReport<EffectOperation_Ext>(effectBlock, 1);

    int forces[2] = {0};

    const int testPosition = (USB_AXIS_MAX_ABSOLUTE * 3) / 4;

    UpdatePosition({testPosition / 2, testPosition / 2});
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
    EXPECT_EQ(forces[1], 0);

    UpdatePosition({testPosition, testPosition});
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], -63);
    EXPECT_EQ(forces[1], -63);

    UpdatePosition({USB_AXIS_MAX_ABSOLUTE, USB_AXIS_MAX_ABSOLUTE});
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], -127);
    EXPECT_EQ(forces[1], -127);

    UpdatePosition({-USB_AXIS_MAX_ABSOLUTE, -USB_AXIS_MAX_ABSOLUTE});
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 255);
    EXPECT_EQ(forces[1], 255);
}

TEST_F(HidAbstractor, TestConditionDirectionEnable)
{
    int effectBlock = CreateEffect(
        USB_EFFECT_INERTIA,
        USB_DURATION_INFINITE,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        DIRECTION_ENABLE,
        USB_RAD_45,
        0,
        ZERO_START_DELAY);

    SetReport<SetCondition_Ext>(effectBlock, 0, 0, USB_MAX_GAIN, USB_MAX_GAIN, USB_MAX_GAIN, USB_MAX_GAIN, 0);
    SetReport<EffectOperation_Ext>(effectBlock, 1);

    int forces[2] = {0};

    UpdatePosition({-USB_AXIS_MAX_ABSOLUTE, USB_AXIS_MAX_ABSOLUTE});
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
    EXPECT_EQ(forces[1], 0);

    UpdatePosition({USB_AXIS_MAX_ABSOLUTE, -USB_AXIS_MAX_ABSOLUTE});
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
    EXPECT_EQ(forces[1], 0);

    UpdatePosition({0, 0});

    UpdatePosition({USB_AXIS_MAX_ABSOLUTE, USB_AXIS_MAX_ABSOLUTE});
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], -180);
    EXPECT_EQ(forces[1], -180);

    UpdatePosition({0, 0});
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 180);
    EXPECT_EQ(forces[1], 180);

    UpdatePosition({-USB_AXIS_MAX_ABSOLUTE, -USB_AXIS_MAX_ABSOLUTE});
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
    EXPECT_EQ(forces[1], 0);
}

TEST_F(HidAbstractor, TestRampForce)
{
    ResetFakeTime();
    int effectBlock = CreateEffect(
        USB_EFFECT_RAMP,
        4,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetRampForce_Ext>(effectBlock, 60, 100);
    SetReport<EffectOperation_Ext>(effectBlock, 1);

    int forces[2] = {0};
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 60);
    TickFakeTime();

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 70);
    TickFakeTime();

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 80);
    TickFakeTime();

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 90);
    TickFakeTime();

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
    TickFakeTime();
}

TEST_F(HidAbstractor, TestTriangleWave)
{
    ResetFakeTime();
    int test_samples = 100;
    int effectBlock = CreateEffect(
        USB_EFFECT_TRIANGLE,
        test_samples,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetPeriodic_Ext>(effectBlock, 100, 1, 0, test_samples);
    SetReport<EffectOperation_Ext>(effectBlock, 1);
    int forces[2] = {0};
    int forceSum = 0;
    for (int i = 0; i < test_samples; ++i)
    {
        ffe->ForceCalculator(forces);
        forceSum += forces[0];
        TickFakeTime();
    }

    EXPECT_EQ(forceSum, 100);
}

TEST_F(HidAbstractor, TestSawtoothUpDownSimultaneous)
{
    int test_samples = 1000;

    ResetFakeTime();

    int effectBlock = CreateEffect(
        USB_EFFECT_SAWTOOTHUP,
        test_samples,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetPeriodic_Ext>(effectBlock, 100, 0, 0, test_samples);
    SetReport<EffectOperation_Ext>(effectBlock, 1);

    effectBlock = CreateEffect(
        USB_EFFECT_SAWTOOTHDOWN,
        test_samples,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetPeriodic_Ext>(effectBlock, 100, 0, 0, test_samples);
    SetReport<EffectOperation_Ext>(effectBlock, 1);

    int forces[2] = {0};
    for (int i = 0; i < test_samples; ++i)
    {
        ffe->ForceCalculator(forces);

        int forceSum = std::accumulate(forces, forces + NUM_AXES, 0);
        EXPECT_EQ(forceSum, 100);

        TickFakeTime();
    }
}

TEST_F(HidAbstractor, TestPeriodicPhase)
{
    ResetFakeTime();
    int test_samples = 100;
    for (int phase = 0; phase < USB_MAX_PHASE; phase += 10)
    {
        int effectBlock = CreateEffect(
            USB_EFFECT_SINE,
            test_samples,
            ZERO_TRIGGER_REPEAT_INTERVAL,
            ZERO_SAMPLE_INTERVAL,
            USB_MAX_GAIN,
            USB_NO_TRIGGER_BUTTON,
            X_AXIS_ENABLE,
            0,
            0,
            ZERO_START_DELAY);

        SetReport<SetPeriodic_Ext>(effectBlock, 100, 0, phase, test_samples);
        SetReport<EffectOperation_Ext>(effectBlock, 1);
        int forces[2] = {0};
        int forceSum = 0;
        for (int i = 0; i < test_samples; ++i)
        {
            ffe->ForceCalculator(forces);
            forceSum += forces[0];
            TickFakeTime();
        }

        EXPECT_EQ(forceSum, 0) << "Phase " << phase << std::endl;
        SetReport<BlockFree_Ext>(effectBlock);
    }
}

TEST_F(HidAbstractor, TestSineWave)
{
    ResetFakeTime();
    int test_samples = 100;
    int effectBlock = CreateEffect(
        USB_EFFECT_SINE,
        test_samples,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetPeriodic_Ext>(effectBlock, 100, 1, 0, test_samples);
    SetReport<EffectOperation_Ext>(effectBlock, 1);
    int forces[2] = {0};
    int forceSum = 0;
    for (int i = 0; i < test_samples; ++i)
    {
        ffe->ForceCalculator(forces);
        forceSum += forces[0];
        TickFakeTime();
    }

    EXPECT_EQ(forceSum, 99);
}

TEST_F(HidAbstractor, TestSquareWave)
{
    ResetFakeTime();

    int effectBlock = CreateEffect(
        USB_EFFECT_SQUARE,
        2,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetPeriodic_Ext>(effectBlock, 1, 0, 0, 2);
    SetReport<EffectOperation_Ext>(effectBlock, 1);
    int forces[2] = {0};

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 1);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], -1);
}

TEST_F(HidAbstractor, TestConstantEnvelope)
{
    ResetFakeTime();

    int effectBlock = CreateEffect(
        USB_EFFECT_CONSTANT,
        5,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetConstantForce_Ext>(effectBlock, 100);
    SetReport<SetEnvelope_Ext>(effectBlock, 0, 0, 2, 2);
    SetReport<EffectOperation_Ext>(effectBlock, 1);
    int forces[2] = {0};

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 50);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 100);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 100);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 50);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
}

TEST_F(HidAbstractor, TestConstantEnvelopeFade)
{
    ResetFakeTime();

    int effectBlock = CreateEffect(
        USB_EFFECT_CONSTANT,
        8,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetConstantForce_Ext>(effectBlock, USB_MAX_MAGNITUDE);
    SetReport<SetEnvelope_Ext>(effectBlock, 1, 0, 0, 8);
    SetReport<EffectOperation_Ext>(effectBlock, 1);
    int forces[2] = {0};

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], USB_MAX_MAGNITUDE);

    SetFakeTime(3);
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 159);

    SetFakeTime(6);
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 63);

    SetFakeTime(7);
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 31);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
}

TEST_F(HidAbstractor, TestConstantXSolo)
{
    ResetFakeTime();

    int effectBlock = CreateEffect(
        USB_EFFECT_CONSTANT,
        1,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        X_AXIS_ENABLE,
        0,
        0,
        ZERO_START_DELAY);

    SetReport<SetConstantForce_Ext>(effectBlock, 1);
    SetReport<EffectOperation_Ext>(effectBlock, 1);
    int forces[2] = {0};

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 1);
    EXPECT_EQ(forces[1], 0);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
    EXPECT_EQ(forces[1], 0);
}

TEST_F(HidAbstractor, TestConstantYSolo)
{
    ResetFakeTime();

    int effectBlock = CreateEffect(
        USB_EFFECT_CONSTANT,
        1,
        ZERO_TRIGGER_REPEAT_INTERVAL,
        ZERO_SAMPLE_INTERVAL,
        USB_MAX_GAIN,
        USB_NO_TRIGGER_BUTTON,
        Y_AXIS_ENABLE,
        0,
        USB_RAD_270,
        ZERO_START_DELAY);

    SetReport<SetConstantForce_Ext>(effectBlock, 100);
    SetReport<EffectOperation_Ext>(effectBlock, 1);
    int forces[2] = {0};

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
    EXPECT_EQ(forces[1], -99);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
    EXPECT_EQ(forces[1], 0);
}
