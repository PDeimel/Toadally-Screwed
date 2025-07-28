# Toadally Screwed

---

A Standalone and VST3-Plugin for trying to mimic the one and only Toad from Mario (specifically the New Super Mario Bros. games).

<img src="https://img.shields.io/badge/Language-C%2B%2B20-blue.svg" alt="Language C++20">
<img src="https://img.shields.io/badge/Framework-JUCE-orange.svg" alt="Framework JUCE">
<img src="https://img.shields.io/badge/Build-CMake-green.svg" alt="Build CMake">

**Table of Contents**

1. [🍄 What is Toadally Screwed?](#what-is-toadally-screwed)
2. [🎧 Features](#features)
3. [🏗 Build System](#build-system)
4. [🛠 Dependencies](#dependencies)
5. [💀 License](#license)

---

## What is Toadally Screwed?

"Toadally Screwed" is a Standalone/VST3 synthesizer audio-plugin created with the **JUCE Framework** in combination with **Cmake**.
The goal of this plugin is to create a truly scratchy and unlistenable voice which creates actual vowels, not just basic synthesizer sounds.


---

## Features

* **4 Awesome Oszillators**, each coming with a ready **Preset** to mimic your favourite Toad sound!
* **Live Preview** of the waveform in real-time!
* A complete **ADSR-Envelope** to completely control your sound experience!
* Two **Effects**, a **Reverb** and a **Bitcrusher** to truly simulate the scratchy voice of the mushroom man!
* A mostly functional **Vowel-Slider** which allows to move the sound from a pitched "A" to a deep "U"!
* A neat little **VU-Meter** to at least you show that you are creating a sound!
* Use it **Cross-Platform** as a VST3 Plugin for your favourite DAW!
* Use it as a **Standalone** to unleash your inner Mario, squeezing 'em Shrooms!
---

## Build System

This project uses CMake as the primary build system. To build the project:

1. Clone the repository.
2. Navigate to the project root directory:
   ```bash
   cd Toady
   ```
3. Create a build directory and navigate to it:
   ```bash
   mkdir build && cd build
   ```
4. Configure the project using CMake:
   ```bash
   cmake ..
   ```
5. Build the project:
   ```bash
   cmake --build .
   ```

---

## Dependencies

Toadally Screwed relies on the following key dependencies:

1. **JUCE Framework**: For audio processing and UI management.
2. **magic_enum**: A modern C++ library for enum handling.
3. Additional dependencies for cross-platform support.

Dependencies are managed using `CMake` for seamless integration. Ensure all dependencies are installed or accessible
during the configuration process.

---

## License

Refer to the `LICENSE` files for more details on licensing terms.