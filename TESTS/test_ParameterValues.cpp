#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Processor/PluginProcessor.h"

TEST_CASE("Pan parameter accepts mid-range values", "[Pulsar][Parameters][Pan]")
{
    PulsarAudioProcessor processor;
    auto* param = processor.apvts.getParameter("Pan");
    REQUIRE(param != nullptr);

    // Default should be 50 (center pan)
    float raw = processor.apvts.getRawParameterValue("Pan")->load();
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(50.0f, 0.01f));

    // Set to 25% via normalized value
    // Pan range is 0-100, so 25/100 = 0.25 normalized (skew is 1.0 so linear)
    param->setValueNotifyingHost(0.25f);
    raw = processor.apvts.getRawParameterValue("Pan")->load();
    INFO("Pan set to normalized 0.25, raw value = " << raw);
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(25.0f, 0.1f));

    // Set to 75%
    param->setValueNotifyingHost(0.75f);
    raw = processor.apvts.getRawParameterValue("Pan")->load();
    INFO("Pan set to normalized 0.75, raw value = " << raw);
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(75.0f, 0.1f));

    // Set to exact center
    param->setValueNotifyingHost(0.5f);
    raw = processor.apvts.getRawParameterValue("Pan")->load();
    INFO("Pan set to normalized 0.5, raw value = " << raw);
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(50.0f, 0.1f));
}

TEST_CASE("Amp parameter accepts mid-range values", "[Pulsar][Parameters][Amp]")
{
    PulsarAudioProcessor processor;
    auto* param = processor.apvts.getParameter("Amp");
    REQUIRE(param != nullptr);

    param->setValueNotifyingHost(0.3f);
    float raw = processor.apvts.getRawParameterValue("Amp")->load();
    INFO("Amp set to normalized 0.3, raw value = " << raw);
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(30.0f, 0.2f));

    param->setValueNotifyingHost(0.7f);
    raw = processor.apvts.getRawParameterValue("Amp")->load();
    INFO("Amp set to normalized 0.7, raw value = " << raw);
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(70.0f, 0.2f));
}

TEST_CASE("Fundamental Freq parameter accepts mid-range values", "[Pulsar][Parameters][Fund]")
{
    PulsarAudioProcessor processor;
    auto* param = processor.apvts.getParameter("Fundamental Freq");
    REQUIRE(param != nullptr);

    // Range is 1-200, skew 0.5 (non-linear)
    // With skew 0.5, normalized 0.5 maps to sqrt(0.5)*(200-1)+1 ≈ ~141.7 (not 100.5)
    // Let's just verify it's not pinned to 1 or 200
    param->setValueNotifyingHost(0.5f);
    float raw = processor.apvts.getRawParameterValue("Fundamental Freq")->load();
    INFO("Fund Freq set to normalized 0.5, raw value = " << raw);
    REQUIRE(raw > 1.0f);
    REQUIRE(raw < 200.0f);

    param->setValueNotifyingHost(0.25f);
    raw = processor.apvts.getRawParameterValue("Fundamental Freq")->load();
    INFO("Fund Freq set to normalized 0.25, raw value = " << raw);
    REQUIRE(raw > 1.0f);
    REQUIRE(raw < 200.0f);
}

TEST_CASE("Envelope parameters accept mid-range values", "[Pulsar][Parameters][Envelope]")
{
    PulsarAudioProcessor processor;

    auto* attack = processor.apvts.getParameter("Attack");
    auto* sustain = processor.apvts.getParameter("Sustain Level");
    REQUIRE(attack != nullptr);
    REQUIRE(sustain != nullptr);

    attack->setValueNotifyingHost(0.5f);
    float raw = processor.apvts.getRawParameterValue("Attack")->load();
    INFO("Attack set to normalized 0.5, raw value = " << raw);
    REQUIRE(raw > 0.0f);
    REQUIRE(raw < 10000.0f);

    sustain->setValueNotifyingHost(0.5f);
    raw = processor.apvts.getRawParameterValue("Sustain Level")->load();
    INFO("Sustain set to normalized 0.5, raw value = " << raw);
    REQUIRE_THAT(raw, Catch::Matchers::WithinAbs(0.5f, 0.02f));
}
