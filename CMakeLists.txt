# Require minimum CMake version 3.30
cmake_minimum_required(VERSION 3.30)

# Define the project name
project(Toady VERSION 0.1.0)

# Set C++23 as the required standard for all targets
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Use fetch content to download magic_enum
include(FetchContent)
FetchContent_Declare(
        magic_enum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
        GIT_TAG v0.9.7
)
FetchContent_MakeAvailable(magic_enum)

FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.8
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Toady
        # VERSION ...                               # Set this if the plugin version is different to the project version
        # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
        # ICON_SMALL ...
        COMPANY_NAME "Nintendon't"                         # Specify the name of the plugin's author
        IS_SYNTH TRUE                               # Is this a synth or an effect?
        NEEDS_MIDI_INPUT TRUE                       # Does the plugin need midi input?
        NEEDS_MIDI_OUTPUT FALSE                     # Does the plugin need midi output?
        IS_MIDI_EFFECT FALSE                        # Is this plugin a MIDI effect?
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE           # Does the editor need keyboard focus?
        COPY_PLUGIN_AFTER_BUILD TRUE                # Should the plugin be installed to a default location after building?
        PLUGIN_MANUFACTURER_CODE Hsbi               # A four-character manufacturer id with at least one upper-case character
        PLUGIN_CODE Avsy                            # A unique four-character plugin id with exactly one upper-case character
        # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
        FORMATS VST3 Standalone                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
        PRODUCT_NAME "Toadally Screwed")    # The name of the final executable, which can differ from the target name

# `juce_generate_juce_header` will create a JuceHeader.h for a given target, which will be generated
# into your build tree. This should be included with `#include <JuceHeader.h>`. The include path for
# this header will be automatically added to the target. The main function of the JuceHeader is to
# include all your JUCE module headers; if you're happy to include module headers directly, you
# probably don't need to call this.

juce_generate_juce_header(Toady)

# Binary data für Assets (Bilder, etc.)
juce_add_binary_data(ToadyAssets
        SOURCES
        assets/images/sine_wave.png
        assets/images/square_wave.png
        assets/images/sawtooth_wave.png
        assets/images/triangle_wave.png
        NAMESPACE ToadyAssets
)

# `target_sources` adds source files to a target. We pass the target that needs the sources as the
# first argument, then a visibility parameter for the sources which should normally be PRIVATE.
# Finally, we supply a list of source files that will be built into the target. This is a standard
# CMake command.

target_sources(Toady
        PRIVATE
        src/ADSRComponent.cpp
        src/ADSRComponent.hpp
        src/PluginEditor.cpp
        src/PluginProcessor.cpp
        src/WaveformComponent.cpp
        src/Utils.cpp)

# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.

target_compile_definitions(Toady
        PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
        DONT_SET_USING_JUCE_NAMESPACE
)

# If your target needs extra binary assets, you can add them here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when your target is built.

# juce_add_binary_data(AudioPluginData SOURCES ...)

# `target_link_libraries` links libraries and JUCE modules to other libraries or executables. Here,
# we're linking our executable target to the `juce::juce_audio_utils` module. Inter-module
# dependencies are resolved automatically, so `juce_core`, `juce_events` and so on will also be
# linked automatically. If we'd generated a binary data target above, we would need to link to it
# here too. This is a standard CMake command.

target_link_libraries(Toady
        PRIVATE
        ToadyAssets
        juce::juce_audio_utils
        juce::juce_dsp
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags

        # magic_enum
        magic_enum::magic_enum
)