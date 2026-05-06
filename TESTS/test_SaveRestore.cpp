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

    const auto values = RD::ParamHelper::getParamValues(apvts, Pulsar::kFundamentalFreqID);

    REQUIRE_THAT(values.worldValue, Catch::Matchers::WithinAbs(targetWorldValue, 1.0e-3f));
}
