cmake_minimum_required(VERSION 3.24.1)

# Read version from VERSION.txt
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt" VERSION_STRING)
string(STRIP "${VERSION_STRING}" VERSION_STRING)

# Parse version components
string(REPLACE "." ";" VERSION_LIST ${VERSION_STRING})
list(GET VERSION_LIST 0 PROJECT_VERSION_MAJOR)
list(GET VERSION_LIST 1 PROJECT_VERSION_MINOR)
list(GET VERSION_LIST 2 PROJECT_VERSION_PATCH)

# Project definition
project(Pulsar VERSION ${VERSION_STRING} LANGUAGES C CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# JUCE setup
add_subdirectory(SUBMODULES/JUCE)

# Plugin formats to build
set(FORMATS VST3 Standalone)

# Create version header before building
add_custom_target(update_version_header
    COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/HELPER_SCRIPTS/update_version.py"
            "${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt"
            "${CMAKE_CURRENT_SOURCE_DIR}/SOURCE/Util/Version.h"
    BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/SOURCE/Util/Version.h"
    COMMENT "Updating version header"
)

# Plugin configuration
juce_add_plugin(Pulsar
    COMPANY_NAME "recluse-audio"
    BUNDLE_ID com.recluseaudio.pulsar
    PLUGIN_MANUFACTURER_CODE Rcls
    PLUGIN_CODE Plsr
    FORMATS ${FORMATS}
    PRODUCT_NAME "Pulsar"
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_SYNTH TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Synth
)

# Generate JuceHeader.h for legacy #include <JuceHeader.h> compatibility
juce_generate_juce_header(Pulsar)

# Source files
include(CMAKE/SOURCES.cmake)
target_sources(Pulsar PRIVATE ${SOURCES})

# Include directories
target_include_directories(Pulsar PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/SOURCE
)

# Make version header generation a dependency
add_dependencies(Pulsar update_version_header)

# Compiler options
target_compile_features(Pulsar PRIVATE cxx_std_20)

# Link JUCE modules
target_link_libraries(Pulsar
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Preprocessor definitions
target_compile_definitions(Pulsar
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_REPORT_APP_USAGE=0
)

# Platform-specific settings
if(MSVC)
    target_compile_options(Pulsar PRIVATE /W4 /wd4244)
else()
    target_compile_options(Pulsar PRIVATE -Wall -Wextra -Wpedantic)
endif()

#==============================================================================
# Tests (optional)
#==============================================================================
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS OR TARGET Tests)
    include(FetchContent)

    # Fetch Catch2
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.1.0
    )
    FetchContent_MakeAvailable(Catch2)

    # Test executable
    add_executable(Tests)
    include(CMAKE/TESTS.cmake)
    include(CMAKE/SOURCES.cmake)
    target_sources(Tests PRIVATE ${TEST_SOURCES} ${SOURCES})

    target_include_directories(Tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/SOURCE
        ${CMAKE_BINARY_DIR}/Pulsar_artefacts/JuceLibraryCode
    )

    target_link_libraries(Tests PRIVATE
        Catch2::Catch2WithMain
        juce::juce_audio_utils
        juce::juce_audio_processors
    )

    target_compile_features(Tests PRIVATE cxx_std_20)

    # MSVC: Add /FS flag to allow parallel compilation with shared PDB
    if(MSVC)
        target_compile_options(Tests PRIVATE /FS)
    endif()

    # Define JUCE plugin macros for Tests target
    target_compile_definitions(Tests PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_REPORT_APP_USAGE=0
        JucePlugin_Name="Pulsar"
        JucePlugin_WantsMidiInput=1
        JucePlugin_ProducesMidiOutput=0
        JucePlugin_IsMidiEffect=0
        JucePlugin_IsSynth=1
    )

    # Ensure JuceHeader.h is generated before building Tests
    add_dependencies(Tests Pulsar)

    # Enable CTest
    include(CTest)
    include(Catch)
    catch_discover_tests(Tests)
endif()
