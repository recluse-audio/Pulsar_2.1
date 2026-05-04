#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "TestUtils.h"
#include "Processor/PluginProcessor.h"

TEST_CASE("Pan parameter accepts mid-range values", "[Pulsar][Parameters][Pan]")
{
    TestUtils::SetupAndTeardown setup;

    PulsarAudioProcessor processor;
    auto* param = processor.apvts.getParameter(Pulsar::kPanID);
    REQUIRE(param != nullptr);

    float raw = processor.apvts.getRawParameterValue(Pulsar::kPanID)->load();
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(50.0f, 0.01f));

    param->setValueNotifyingHost(0.25f);
    raw = processor.apvts.getRawParameterValue(Pulsar::kPanID)->load();
    INFO("Pan set to normalized 0.25, raw value = " << raw);
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(25.0f, 0.1f));

    param->setValueNotifyingHost(0.75f);
    raw = processor.apvts.getRawParameterValue(Pulsar::kPanID)->load();
    INFO("Pan set to normalized 0.75, raw value = " << raw);
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(75.0f, 0.1f));

    param->setValueNotifyingHost(0.5f);
    raw = processor.apvts.getRawParameterValue(Pulsar::kPanID)->load();
    INFO("Pan set to normalized 0.5, raw value = " << raw);
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(50.0f, 0.1f));
}

TEST_CASE("Amp parameter accepts mid-range values", "[Pulsar][Parameters][Amp]")
{
    TestUtils::SetupAndTeardown setup;

    PulsarAudioProcessor processor;
    auto* param = processor.apvts.getParameter(Pulsar::kAmpID);
    REQUIRE(param != nullptr);

    param->setValueNotifyingHost(0.3f);
    float raw = processor.apvts.getRawParameterValue(Pulsar::kAmpID)->load();
    INFO("Amp set to normalized 0.3, raw value = " << raw);
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(30.0f, 0.2f));

    param->setValueNotifyingHost(0.7f);
    raw = processor.apvts.getRawParameterValue(Pulsar::kAmpID)->load();
    INFO("Amp set to normalized 0.7, raw value = " << raw);
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(70.0f, 0.2f));
}

TEST_CASE("Fundamental Freq parameter accepts mid-range values", "[Pulsar][Parameters][Fund]")
{
    TestUtils::SetupAndTeardown setup;

    PulsarAudioProcessor processor;
    auto* param = processor.apvts.getParameter(Pulsar::kFundamentalFreqID);
    REQUIRE(param != nullptr);

    param->setValueNotifyingHost(0.5f);
    float raw = processor.apvts.getRawParameterValue(Pulsar::kFundamentalFreqID)->load();
    INFO("Fund Freq set to normalized 0.5, raw value = " << raw);
    REQUIRE(raw > 1.0f);
    REQUIRE(raw < 200.0f);

    param->setValueNotifyingHost(0.25f);
    raw = processor.apvts.getRawParameterValue(Pulsar::kFundamentalFreqID)->load();
    INFO("Fund Freq set to normalized 0.25, raw value = " << raw);
    REQUIRE(raw > 1.0f);
    REQUIRE(raw < 200.0f);
}

TEST_CASE("Envelope parameters accept mid-range values", "[Pulsar][Parameters][Envelope]")
{
    TestUtils::SetupAndTeardown setup;

    PulsarAudioProcessor processor;

    auto* attack = processor.apvts.getParameter(Pulsar::kAttackID);
    auto* sustain = processor.apvts.getParameter(Pulsar::kSustainLevelID);
    REQUIRE(attack != nullptr);
    REQUIRE(sustain != nullptr);

    attack->setValueNotifyingHost(0.5f);
    float raw = processor.apvts.getRawParameterValue(Pulsar::kAttackID)->load();
    INFO("Attack set to normalized 0.5, raw value = " << raw);
    REQUIRE(raw > 0.0f);
    REQUIRE(raw < 10000.0f);

    sustain->setValueNotifyingHost(0.5f);
    raw = processor.apvts.getRawParameterValue(Pulsar::kSustainLevelID)->load();
    INFO("Sustain set to normalized 0.5, raw value = " << raw);
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(0.5f, 0.02f));
}
