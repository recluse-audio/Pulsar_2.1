# Pulsar

A JUCE-based pulsar synthesis plugin (VST3 / Standalone) by recluse-audio.

## Building

### Prerequisites
- CMake 3.24.1 or higher
- C++20 compatible compiler
- JUCE framework (included as submodule)

### Setup

If cloning from an existing repository:
```bash
# Clone with submodules
git clone --recursive <repository-url>

# Or if already cloned, initialize submodules
git submodule update --init --recursive
```

### Build Commands

#### All Targets (Clean Rebuild)
```bash
python HELPER_SCRIPTS/rebuild_all.py
```

#### VST3
```bash
python HELPER_SCRIPTS/build_vst3.py
```

#### Run Tests
```bash
python HELPER_SCRIPTS/build_tests.py
cd BUILD
ctest
```

### Build Configuration

By default, builds are created in Debug mode. To build in Release mode:
```bash
# On Windows (multi-config generator)
python HELPER_SCRIPTS/rebuild_all.py --config Release

# On macOS/Linux (single-config generator)
CMAKE_BUILD_TYPE=Release python HELPER_SCRIPTS/rebuild_all.py
```

## Project Structure

```
Pulsar_2.1/
├── SOURCE/              # Plugin source code
│   ├── Processor/       # PluginProcessor, TempoSync
│   ├── Components/      # PluginEditor, UI components
│   ├── Envelope/        # Envelope DSP
│   ├── LookNFeel/       # Custom JUCE LookAndFeel
│   ├── PULSAR_TRAIN/    # Pulsar train generation
│   ├── Pulsaret/        # Pulsaret core, tables, MIDI, factory
│   └── Util/            # Version header
├── CMAKE/               # CMake configuration files
├── HELPER_SCRIPTS/      # Build scripts
├── TESTS/               # Unit tests (Catch2)
├── SUBMODULES/          # Git submodules (JUCE)
├── BUILD/               # Build artifacts (not tracked)
├── CMakeLists.txt       # Root CMake configuration
└── VERSION.txt          # Semantic version
```

## Development

### Version Management
Version is tracked in `VERSION.txt`. The build system automatically generates
`SOURCE/Util/Version.h` with version macros.

### Adding Source Files
1. Add your .h/.cpp files to the `SOURCE/` directory
2. Run `python HELPER_SCRIPTS/regenSource.py` to update `CMAKE/SOURCES.cmake`
3. Rebuild

### Adding Tests
1. Add test files to `TESTS/` directory
2. Run `python HELPER_SCRIPTS/regenSource.py` to update `CMAKE/TESTS.cmake`
3. Run `python HELPER_SCRIPTS/build_tests.py`

## License

[Add your license here]
