# Project README

## Overview
This project is a C-based application designed to read and play audio files. It includes support for various operating systems, including Linux, Windows, Wine, and WebAssembly.

## Features
- Reading of WAV audio files.
- Reading and writing MP3 audio files on Linux through `libmpg123` and `libmp3lame`.
- MP3 input is decoded to interleaved 16-bit PCM and can use the existing WAV/audio playback path.
- Playback of WAV audio files.
- Support for multiple platforms: Linux, Windows, Wine, and WebAssembly.

## Project Structure
```
/home/codeleaded/Hecke/C/Cmd_Audio/
├── build/              # Compiled .exe files produced by Main.c
├── src/                # Source code
│   ├── Main.c          # Entry point
│   └── *.h             # Header files used by Main.c
├── Makefile.linux      # Linux Build configuration
├── Makefile.windows    # Windows Build configuration
├── Makefile.wine       # Wine Build configuration for cross-compiling to Windows
├── Makefile.web        # Emscripten Build configuration for WebAssembly
└── README.md           # This file
```

### Prerequisites
- C/C++ Compiler and Debugger (GCC, Clang)
- Make utility
- Standard development tools
- Linux MP3 runtime libraries: `libmpg123.so.0` and `libmp3lame.so.0`

## Build & Run
To build and run the project, follow these steps:

1. Navigate to the project directory:
    ```sh
    cd /home/codeleaded/Hecke/C/Cmd_Audio/
    ```

2. Build for Linux (Ubuntu):
    ```sh
    make -f Makefile.linux all
    ```

3. Build for Windows:
    ```sh
    make -f Makefile.windows all
    ```

4. Build for Wine (Linux cross-compiling to Windows):
    ```sh
    make -f Makefile.wine all
    ```

5. Build for WebAssembly:
    ```sh
    make -f Makefile.web all
    ```

6. Run the executable:
    ```sh
    make -f Makefile.(os) exe
    ```

7. Clean the build artifacts:
    ```sh
    make -f Makefile.(os) clean
    ```

Replace `(os)` with the appropriate OS specific makefile target (linux, windows, wine, web).

An MP3 file can be played by passing its path to the executable:

```sh
./build/Main ./data/example.mp3
```