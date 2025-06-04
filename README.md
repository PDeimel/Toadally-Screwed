# Project Documentation

## Project Overview

Toadally Screwed wird wunderschön klingen

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