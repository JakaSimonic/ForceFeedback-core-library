#include <gtest/gtest.h>
#include "../UserInput.h"
#include "../FfbEngine.h"
#include "../FfbReportHandler.h"
#include "../HIDReportType.h"
#include <stdint.h>
#include <stdio.h>
#include <memory>
#include "helpers/hidTypesExt.hpp"

uint64_t current_time = 0;

uint64_t GetFakeTime()
{
    return current_time;
}
void TickFakeTime(unsigned int time = 1, bool reset = false)
{
    current_time = time;
}
void ResetFakeTime()
{
    current_time = 0;
}
class HidAbstractor : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ffh = new FfbReportHandler(GetFakeTime);
        ffe = new FfbEngine(*ffh, ui, GetFakeTime);
    }

    UserInput ui;
    FfbReportHandler *ffh = nullptr;
    FfbEngine *ffe = nullptr;

    template <typename... Args>
    int CreateEffect(Args... args)
    {
        ffh->FfbOnCreateNewEffect((USB_FFBReport_CreateNewEffect_Feature_Data_t *)0);
        int effectBlock = ffh->FfbOnPIDBlockLoad()[1];
        std::cout << "Effect block index " << effectBlock << std::endl;

        auto effect = Reportfactory<USB_FFBReport_SetEffect_Output_Data_t, SetEffect_Ext>(effectBlock, args...);
        ffh->FfbOnUsbData((uint8_t *)effect.get(), sizeof(USB_FFBReport_SetEffect_Output_Data_t));

        return effectBlock;
    }

    template <typename T, typename Init, typename... Args>
    std::unique_ptr<T> Reportfactory(Args... args)
    {
        Init *report = new Init(args...);
        return std::unique_ptr<T>(dynamic_cast<T *>(report));
    }

    template <typename T, typename Init, typename... Args>
    void SetReport(Args... args)
    {
        auto effect = Reportfactory<T, Init>(args...);
        ffh->FfbOnUsbData((uint8_t *)effect.get(), sizeof(T));
    }
};

TEST_F(HidAbstractor, TestConstant)
{
    ResetFakeTime();

    int effectBlock = CreateEffect(USB_EFFECT_CONSTANT, 1, 0, 1, 255, 0xFF, X_AXIS_ENABLE, 0, 0, 0);
    SetReport<USB_FFBReport_SetConstantForce_Output_Data_t, SetConstantForce_Ext>(effectBlock, 1);
    SetReport<USB_FFBReport_EffectOperation_Output_Data_t, EffectOperation_Ext>(effectBlock, 1, 0);
    int forces[2] = {0};

    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 1);
    EXPECT_EQ(forces[1], 0);

    TickFakeTime();
    ffe->ForceCalculator(forces);
    EXPECT_EQ(forces[0], 0);
    EXPECT_EQ(forces[1], 0);
}
