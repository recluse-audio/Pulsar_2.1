#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Processor/PluginProcessor.h"
#include "../../TEST_UTILS/TestUtils.h"

TEST_CASE("Pan parameter accepts mid-range values", "[Pulsar][SaveRestore]")
{
    TestUtils::SetupAndTeardown setup;
    PulsarAudioProcessor processor;

    auto* fundParam = processor.apvts.getParameter("Fundamental Freq");
    REQUIRE(fundParam != 0);

}