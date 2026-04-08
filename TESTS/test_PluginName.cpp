#include <catch2/catch_test_macros.hpp>
#include <string>

TEST_CASE("Plugin name is Pulsar", "[Pulsar][PluginName]")
{
    std::string name = JucePlugin_Name;
    REQUIRE(name == "Pulsar");
}
