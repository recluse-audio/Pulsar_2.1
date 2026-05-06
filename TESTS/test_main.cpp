#include <catch2/catch_session.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

int main(int argc, char* argv[])
{
    // Owns JUCE's GUI/event singletons (MessageManager, TimerThread, ShutdownDetector)
    // for the whole process. Required so tests using AudioProcessor / Timer don't
    // leak ShutdownDetector and don't trip JUCE_ASSERT_MESSAGE_MANAGER_EXISTS during
    // teardown. Per-test TestUtils::SetupAndTeardown still marks the message thread.
    juce::ScopedJuceInitialiser_GUI juceInit;

    return Catch::Session().run(argc, argv);
}
