# Toadally Screwed

---

A Standalone and VST3-Plugin for trying to mimic the one and only Toad from Mario (specifically the New Super Mario Bros. games).

<img src="https://img.shields.io/badge/Language-C%2B%2B20-blue.svg" alt="Language C++20">
<img src="https://img.shields.io/badge/Framework-JUCE-orange.svg" alt="Framework JUCE">
<img src="https://img.shields.io/badge/Build-CMake-green.svg" alt="Build CMake">
<img src="https://img.shields.io/badge/CMake-3.30%2B-red.svg" alt="CMake 3.30+">

**Table of Contents**

1. [🍄 What is Toadally Screwed?](#what-is-toadally-screwed)
2. [🎧 Features](#features)
3. [🏗 Build System](#build-system)
4. [🛠 Dependencies](#dependencies)
5. [📋 Requirements](#requirements)
6. [🚀 Installation](#installation)
7. [🎵 Usage](#usage)
8. [📁 Project Structure](#project-structure)
9. [💀 License](#license)

---

## What is Toadally Screwed?

"Toadally Screwed" is a Standalone/VST3 synthesizer audio-plugin created with the **JUCE Framework** in combination with **CMake**.
The goal of this plugin is to create a truly scratchy and unlistenable voice which creates actual vowels, not just basic synthesizer sounds.

---

## Features

* **4 Awesome Oszillators**, each coming with a ready **Preset** to mimic your favourite Toad sound!
* **Live Preview** of the waveform in real-time!
* A complete **ADSR-Envelope** to completely control your sound experience!
* Two **Effects**, a **Reverb** and a **Bitcrusher** to truly simulate the scratchy voice of the mushroom man!
* A mostly functional **Vowel-Slider** which allows to move the sound from a pitched "A" to a deep "U"!
* A neat little **VU-Meter** to at least show you that you are creating a sound!
* Use it **Cross-Platform** as a VST3 Plugin for your favourite DAW!
* Use it as a **Standalone** to unleash your inner Mario, squeezing 'em Shrooms!

---

## Requirements

### System Requirements
- **CMake 3.30** or higher
- **C++20** compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- Git (for dependency fetching)

### Platform Support
- Windows (VST3, Standalone)
- macOS (VST3, Standalone)
- Linux (VST3, Standalone)

---

## Build System

This project uses CMake as the primary build system with automatic dependency management via FetchContent.

### Quick Build

1. **Clone the repository:**
   ```bash
   git clone <repository-url>
   cd Toady
   ```

2. **Create build directory:**
   ```bash
   mkdir build && cd build
   ```

3. **Configure and build:**
   ```bash
   cmake ..
   cmake --build . --config Release
   ```

### Build Configurations

- **Debug Build:**
  ```bash
  cmake .. -DCMAKE_BUILD_TYPE=Debug
  cmake --build .
  ```

- **Release Build:**
  ```bash
  cmake .. -DCMAKE_BUILD_TYPE=Release
  cmake --build .
  ```

### Plugin Installation

With `COPY_PLUGIN_AFTER_BUILD=TRUE`, the plugin will automatically be copied to your system's default plugin directory after building:

- **Windows:** `%APPDATA%/Roaming/VST3/`
- **macOS:** `~/Library/Audio/Plug-Ins/VST3/`
- **Linux:** `~/.vst3/`

---

## Dependencies

All dependencies are automatically downloaded and configured via CMake's FetchContent:

### Automatically Managed Dependencies

1. **JUCE Framework** (v8.0.8)
   - Core audio processing and UI framework
   - Modules: `juce_audio_utils`, `juce_dsp`
   - Repository: https://github.com/juce-framework/JUCE.git

2. **magic_enum** (v0.9.7)
   - Modern C++ enum reflection library
   - Repository: https://github.com/Neargye/magic_enum.git

### System Dependencies

Ensure you have the following installed on your system:

**Windows:**
- Visual Studio 2019 or newer (with C++ support)
- Windows 10 SDK

**macOS:**
- Xcode 12 or newer
- macOS 10.13 or newer

**Linux:**
- GCC 10+ or Clang 10+
- Development packages: `build-essential`, `libasound2-dev`, `libfreetype6-dev`, `libx11-dev`, `libxcomposite-dev`, `libxcursor-dev`, `libxext-dev`, `libxinerama-dev`, `libxrandr-dev`, `libxrender-dev`

**Install Linux dependencies:**
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential libasound2-dev libfreetype6-dev libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev

# Fedora
sudo dnf install gcc-c++ alsa-lib-devel freetype-devel libX11-devel libXcomposite-devel libXcursor-devel libXext-devel libXinerama-devel libXrandr-devel libXrender-devel
```

---

## Installation

### Option 1: Build from Source (Recommended)

Follow the [Build System](#build-system) instructions above.

### Option 2: Manual Plugin Installation

If you have a pre-built plugin:

1. Copy `Toadally Screwed.vst3` to your VST3 plugin directory
2. Copy the Standalone executable to your desired location

---

## Usage

### As VST3 Plugin
1. Open your favorite DAW (Reaper, Ableton, FL Studio, etc.)
2. Scan for new plugins or manually load "Toadally Screwed"
3. Create a MIDI track and load the plugin
4. Play MIDI notes to trigger Toad sounds

### As Standalone Application
1. Run the `Toadally Screwed` executable
2. Configure your audio device in the settings
3. Use your MIDI keyboard or the on-screen keyboard
4. Adjust parameters in real-time

### Controls
- **Waveform Selection:** Choose between Sine, Square, Sawtooth, and Triangle waves
- **ADSR Envelope:** Control Attack, Decay, Sustain, and Release
- **Vowel Filter:** Morph between vowel sounds (A to U)
- **Effects:** Add Reverb and Bitcrusher for that authentic scratchy Toad voice
- **VU Meter:** Monitor output levels

---

## Project Structure

```
Toady/
├── CMakeLists.txt              # Main CMake configuration
├── README.md                   # This file
├── LICENSE                     # License information
├── assets/
│   └── images/                 # Waveform icons
│       ├── ...
└── src/                        
    ├── ...
```

---

## Plugin Information

- **Company:** Nintendon't
- **Plugin Name:** Toadally Screwed
- **Manufacturer Code:** Hsbi
- **Plugin Code:** Toad
- **Version:** 1.0.0
- **Type:** Synthesizer
- **MIDI:** Requires MIDI input, no MIDI output
- **Formats:** VST3, Standalone

---

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

### Development Setup
1. Follow the build instructions above
2. Make your changes
3. Test thoroughly on your platform
4. Create a pull request with a clear description

---

## License

Refer to the `LICENSE` file for more details on licensing terms.

---

## Acknowledgments

- **JUCE Team** for the excellent audio framework
- **Neargye** for the magic_enum library
- **Nintendo** for creating the iconic Toad character (this is a fan project)

---

*"Toadally Screwed" - Because sometimes you just need that perfect mushroom retainer sound! 🍄*