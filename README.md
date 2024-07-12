
# Lie

Lie is a simple, lightweight, and easy-to-use terminal-based text editor written in C.

It is currently in early stages of development and is not yet ready for use.

## Features

- Text viewing with scroll support
- Status bar with error messages
- TBA

## Building

### Requirements

- C17 compiler (GCC, Clang, etc.)
- CMake 3.27 or higher

### Instructions

1. Clone the repository
```console
> git clone https://github.com/alihakankurt/Lie.git
```

2. Move into the root directory
```console
> cd Lie
```

3. Create CMake build files
```console
> cmake -S . -B Build
```

4. Build the project
```console
> cmake --build Build
```

5. Run the executable
```console
> ./Bin/Lie [filename]
```

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.
