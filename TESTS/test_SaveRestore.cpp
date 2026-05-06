#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Processor/PluginProcessor.h"
#include "ParamHelper.h"
#include "TestUtils.h"

TEST_CASE("Fundamental Freq param saves/restores default and modified state", "[Pulsar][SaveRestore]")
{
    TestUtils::SetupAndTeardown setup;
    PulsarAudioProcessor processor;
    AudioProcessorValueTreeState& apvts = processor.apvts;

    auto* fundParam = apvts.getParameter(Pulsar::kFundamentalFreqID);
    REQUIRE(fundParam != nullptr);

    constexpr float targetWorldValue = 50.f;

    REQUIRE(RD::ParamHelper::setParamWorldValue(apvts, Pulsar::kFundamentalFreqID, targetWorldValue));

    const auto result_value_1 = RD::ParamHelper::getParamValues(apvts, Pulsar::kFundamentalFreqID);

    REQUIRE_THAT(result_value_1.worldValue, Catch::Matchers::WithinAbs(targetWorldValue, 1.0e-3f));

    juce::MemoryBlock memoryBlock;
    processor.getStateInformation(memoryBlock);

    // should be able to take that memoryBlock and load it in a new processor right?
    PulsarAudioProcessor processor_2;
    AudioProcessorValueTreeState& apvts_2 = processor_2.apvts;

    // get init value
    auto* fundParam_2 = apvts_2.getParameter(Pulsar::kFundamentalFreqID);
    const auto init_value_2 = RD::ParamHelper::getParamValues(apvts_2, Pulsar::kFundamentalFreqID);

    // set state with mem block from modified processor above
    processor_2.setStateInformation(memoryBlock.getData(), (int)memoryBlock.getSize());

    const auto result_value_2 = RD::ParamHelper::getParamValues(apvts_2, Pulsar::kFundamentalFreqID);
    REQUIRE(init_value_2.worldValue != result_value_2.worldValue);
    REQUIRE(result_value_1.worldValue == result_value_2.worldValue);

}
