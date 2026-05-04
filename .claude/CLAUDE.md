# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Pulsar is a JUCE-based pulsar synthesis plugin (VST3 / AU / Standalone) by recluse-audio. C++20, CMake 3.24.1+. JUCE is vendored as a submodule at `SUBMODULES/JUCE`; shared test/build helpers live in `SUBMODULES/RD`. After a fresh clone run `git submodule update --init --recursive`.

## Common commands

All build entry points are Python scripts under `HELPER_SCRIPTS/`. They all (a) call `regenSource.py` to refresh `CMAKE/SOURCES.cmake` and `CMAKE/TESTS.cmake` and (b) drive CMake against `BUILD/`.

- Full rebuild (Standalone + VST3, plus AU on macOS): `python HELPER_SCRIPTS/rebuild_all.py` (add `--clean` to wipe `BUILD/` first; `--config Release` on Windows/Xcode multi-config).
- VST3 only: `python HELPER_SCRIPTS/build_vst3.py` (defaults to Release).
- Standalone only: `python HELPER_SCRIPTS/build_standalone.py`. AU only (macOS): `python HELPER_SCRIPTS/build_au.py`.
- Tests: `python HELPER_SCRIPTS/build_tests.py` then `cd BUILD && ctest`. The script also clears `TESTS/**/OUTPUT` directories (preserving `.gitkeep`).
- Run a single test by name: `./BUILD/Tests "[PluginName]"` (macOS/Linux) or `./BUILD/Debug/Tests.exe "[Parameters]"` (Windows). Catch2 tag/name filters are passed as the first arg.
- Regenerate file lists without building: `python HELPER_SCRIPTS/regenSource.py`. Run this after adding/removing `.h`/`.cpp` files anywhere under `SOURCE/` or `TESTS/`.
- Single-config generators (Make/Ninja on macOS/Linux) take `CMAKE_BUILD_TYPE` at configure time; multi-config (VS/Xcode) take `--config` at build time. The build scripts handle this branching.

## Architecture

Plugin entry point is `PulsarAudioProcessor` (`SOURCE/Processor/PluginProcessor.{h,cpp}`). It owns a single `PulsarTrain` and forwards parameter changes via `AudioProcessorValueTreeState::Listener`. All parameter IDs are string constants in the `Pulsar` namespace at the top of `PluginProcessor.h` — when adding a parameter, declare its ID there, register it in `createParams()`, and route the listener case in both `PulsarAudioProcessor::parameterChanged` and `PulsarTrain::doParameterChanged` / `PulsaretFactory::doParameterChanged`.

DSP layering, outer to inner:

- `PulsarTrain` (`SOURCE/PULSAR_TRAIN/`) — the per-block scheduler. Owns the ADSR, the tempo sync (`Processor/TempoSync`), two `OwnedPulsaret` voices (`pulsaret1`, `pulsaret2`), and a `PulsaretFactory`. Drives the train period, intermittence/trigger pattern, fundamental glide, panning, and amplitude. Stores parameters in `juce::Atomic` so the audio thread sees changes safely.
- `PulsaretFactory` (`SOURCE/Pulsaret/PulsaretFactory.{h,cpp}`) — recomputes formant frequency, wave selection, duty cycle, and stochastic ranges per pulsaret. Holds two `MidiTransformer` instances for keylock/scale snapping of the two formant bands.
- `OwnedPulsaret` / `Pulsaret` / `PulsaretTable` / `Table` (`SOURCE/Pulsaret/`) — the wavetable lookup that produces per-sample output. `Pulsaret` owns table phase/index; `PulsaretTable` and `Table` hold the waveform data.
- `Envelope` (`SOURCE/Envelope/`) — DSP envelope used independently of the JUCE ADSR in the train.

Editor (`SOURCE/Components/PluginEditor.{h,cpp}`) is a `juce::AudioProcessorEditor` + `juce::Timer`. It polls the processor's `isFlashing()` / `getFlashCoef()` / `isTrainRunning()` to drive UI flashes synchronously with the audio-thread train. Custom look-and-feel classes live in `SOURCE/LookNFeel/`. `PulsaretVisualizer` and `EnvelopeVisualizer` render the underlying tables/envelope.

Threading model: the audio thread reads parameters via `juce::Atomic<T>`; messages from APVTS arrive on the message thread and are forwarded into the train/factory via `doParameterChanged`. Several `std::atomic<bool>` flags (`fundFreqChanged`, `rangesNeedRecalc`, `glideTimeChanged`, `formant1Changed`, `formant2Changed`) defer recalculation to the next audio block.

## CMake structure

`CMakeLists.txt` reads `VERSION.txt`, regenerates `SOURCE/Util/Version.h` via the `update_version_header` custom target (driven by `HELPER_SCRIPTS/update_version.py`), then calls `juce_add_plugin(Pulsar ...)` with formats VST3/AU/Standalone. `Pulsar` is a synth (`IS_SYNTH TRUE`, `NEEDS_MIDI_INPUT TRUE`). Sources are pulled from `CMAKE/SOURCES.cmake` (auto-generated). `juce_generate_juce_header(Pulsar)` produces `<JuceHeader.h>`, which most source files include.

Tests are gated by `-DBUILD_TESTS=ON` (set by `build_tests.py`). The `Tests` target compiles `${TEST_SOURCES} ${SOURCES}` (i.e. all plugin sources are linked into the test binary) and additionally includes `SUBMODULES/RD/TESTS/TEST_UTILS` for shared helpers like `TestUtils::SetupAndTeardown`. `JucePlugin_*` macros are redefined explicitly on the `Tests` target because the JUCE plugin header is generated for the plugin target, not the test executable.

`regenSource.py` deliberately does NOT recurse `SUBMODULES/<x>/SOURCE` — it scans `SOURCE/` only. For tests it picks up `TESTS/` plus `SUBMODULES/<x>/TESTS/TEST_UTILS` (excluding JUCE). If a future submodule needs its sources compiled into Pulsar, edit `discover_source_folders()`.

## Versioning and releases

`VERSION.txt` is the source of truth (currently `1.1.80`). The build target `update_version_header` writes `SOURCE/Util/Version.h` from it — do not edit that header directly. Releases are dispatched per-platform by `release_workflow.py`, which delegates to `SIGNED/release_workflow_{mac,pc}.py`. Windows installers are built by `INSTALLERS/Pulsar.iss` (Inno Setup 6); update its version field to match `VERSION.txt`.
