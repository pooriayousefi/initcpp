# init++ Quick Start Guide

A modern C++ project initializer that cre### Build Types
- `--debug`: Debug build with symbols and no optimization
- `--release`: Release build with optimizations

### Output Types  
- `--executable`: Static executable (default)
- `--static`: Static library (.a)
- `--dynamic`: Dynamic library (.so)l-structured C++ projects with custom utility headers, vcpkg integration, and a command-line build system.

## 🚀 Quick Start

### 1. Build the Initializer
```bash
cd /path/to/init++
g++ -std=c++23 main.cpp -o init++
```

### 2. Create a New Project
```bash
./init++ /path/to/your-new-project
```

### 3. Build Your Project
```bash
cd /path/to/your-new-project
g++ -std=c++23 builder.cpp -o builder
./builder --release --executable
```

### 4. Run Your Project
```bash
./build/release/your_project_name
```

## 📋 What This Tool Does

The `init++` tool creates a complete C++ project structure with:

- **Modern C++23 support**
- **Custom utility headers** (copied from templates: asyncops, RAII filesystem wrappers, string formatters, utilities)
- **vcpkg package manager** (copied from template directory and automatically bootstrapped)
- **Command-line build system** (no CMake/Makefile needed)
- **VSCode configuration** (IntelliSense, tasks, formatting)
- **Multiple build targets** (executable, static lib, dynamic lib)
- **Basic project structure** with organized directories

## 🏗️ Project Structure Created

```
your-project/
├── include/                    # Header files (copied from templates)
│   ├── asyncops.hpp           # Async operations & coroutines
│   ├── raiiiofsw.hpp          # RAII filesystem wrappers
│   ├── stringformers.hpp     # String formatting utilities
│   └── utilities.hpp          # General utility functions
├── src/                       # Source files
│   └── main.cpp               # Main entry point with basic template
├── tests/                     # Test directory (empty, ready for use)
├── build/                     # Build outputs
│   ├── debug/                 # Debug builds
│   └── release/               # Release builds
├── vcpkg/                     # Local vcpkg installation (copied and bootstrapped automatically)
├── .vcpkg-cache/              # Binary package cache (automatic)
├── .vscode/                   # VSCode configuration
│   ├── settings.json          # C++ IntelliSense settings
│   └── tasks.json             # Build tasks
├── builder.cpp                # Build system source code
├── vcpkg.json                 # Package manifest
├── vcpkg-configuration.json   # vcpkg configuration (automatic)
├── install-packages.sh        # Automated package installer
├── update-vcpkg.sh            # vcpkg updater script
└── README.md                  # Project documentation
```

## 🔧 Build System Usage

The generated build system supports multiple configurations:

### Build Types
- `--debug, -d`: Debug build with symbols and no optimization
- `--release, -r`: Release build with optimizations

### Output Types  
- `--executable, --e`: Static executable (default)
- `--static, --s`: Static library (.a)
- `--dynamic, --dy`: Dynamic library (.so)

### Examples
```bash
# Debug executable
./builder --debug --executable

# Release static library
./builder --release --static

# Release dynamic library  
./builder --release --dynamic
```

## 📦 Package Management

### Simple Package Installation

The project includes automated scripts for easy package management:

```bash
# Install packages from vcpkg.json
./install-packages.sh

# Install specific packages directly
./install-packages.sh fmt spdlog nlohmann-json
```

### Adding Dependencies
1. Edit `vcpkg.json` to add packages:
```json
{
  "name": "your-project",
  "version": "1.0.0",
  "dependencies": [
    "fmt",
    "spdlog",
    "nlohmann-json"
  ]
}
```

2. Install packages:
```bash
./install-packages.sh
```

3. The build system automatically includes vcpkg paths

### Updating vcpkg

```bash
# Update vcpkg to latest version and baseline
./update-vcpkg.sh
```

This automatically:
- Updates vcpkg to the latest version
- Updates the baseline in vcpkg.json
- Rebuilds vcpkg executable

## 🎨 Template Headers Included

### `asyncops.hpp`
- Coroutines and async operations
- Generator utilities
- Async task management

### `raiiiofsw.hpp`  
- RAII file and directory wrappers
- Safe filesystem operations
- Automatic resource cleanup

### `stringformers.hpp`
- String formatting utilities
- Type-safe string operations
- Performance-optimized string manipulation

### `utilities.hpp`
- General-purpose utility functions
- Common algorithms and helpers
- Cross-platform compatibility functions

## 🛠️ Development Workflow

### 1. Initial Development
```bash
# Create project
./init++ ~/projects/my-awesome-app

# Navigate and build
cd ~/projects/my-awesome-app
g++ -std=c++23 builder.cpp -o builder

# Start developing
./builder --debug --executable
```

### 2. Adding New Source Files
- Add `.cpp` files to `src/`
- Add `.hpp` files to `include/`
- The build system automatically discovers and compiles them

### 3. Testing
```bash
# Create your own tests in tests/ directory
# No automatic test framework is provided, but directory structure is ready
# You can use any testing framework like Catch2, Google Test, etc.
```

### 4. VSCode Integration
The project includes VSCode configuration for:
- **IntelliSense**: C++23 support with proper include paths
- **Build Tasks**: Ctrl+Shift+P → "Tasks: Run Task"
- **Formatting**: Allman style, format on save
- **Problem Matcher**: Automatic error detection

## 📁 Template Directory Structure

Your `init++` tool requires this structure:

```
init++/
├── main.cpp                   # The initializer source
├── templates/                 # Template files
│   ├── vcpkg/                # Pre-configured vcpkg installation
│   └── include/              # Template header files
│       ├── asyncops.hpp      # Async operations & coroutines
│       ├── raiiiofsw.hpp     # RAII filesystem wrappers
│       ├── stringformers.hpp # String formatting utilities
│       └── utilities.hpp     # General utility functions
├── README.md                 # Main documentation
└── QUICKSTART.md             # This file
```

## 🎯 Use Cases

### 1. Rapid Prototyping
```bash
./init++ ~/prototypes/idea-test
cd ~/prototypes/idea-test
# Edit src/main.cpp with your idea
./builder --debug --executable && ./build/debug/idea_test
```

### 2. Library Development
```bash
./init++ ~/libraries/my-lib
cd ~/libraries/my-lib
# Develop in src/, export in include/
./builder --release --static  # Creates lib*.a
```

### 3. Application Development
```bash
./init++ ~/apps/my-app
cd ~/apps/my-app
# Add dependencies to vcpkg.json, then:
./install-packages.sh
./builder --release --executable
```

## ⚡ Performance Tips

### Fast Builds
- Use `--debug` for development (faster compilation)
- Use `--release` for final builds (optimized)
- The build system uses `-std=c++23` with modern optimizations

### vcpkg Management
- Template vcpkg is automatically copied, bootstrapped, and configured
- Binary caching is enabled by default (stored in `.vcpkg-cache/`)
- Automatic baseline detection and management
- Each project gets its own isolated vcpkg installation
- Use `./install-packages.sh` for easy package installation
- Use `./update-vcpkg.sh` for easy vcpkg updates

## 🔍 Troubleshooting

### Common Issues

**"Template vcpkg not found"**
- Ensure `templates/vcpkg/` exists in your init++ directory
- Copy a working vcpkg installation to `templates/vcpkg/`

**"Template header not found"**
- Verify all `.hpp` files exist in `templates/include/` directory
- Check file permissions (should be readable)

**Build errors**
- Ensure GCC 11+ or Clang 14+ with C++23 support
- Check that all source files have proper syntax
- Verify include paths in build system

**vcpkg package installation fails**
- Run `./install-packages.sh` instead of directly using vcpkg
- Check network connectivity for package downloads
- Verify `vcpkg.json` syntax is correct
- Try `./update-vcpkg.sh` if you suspect vcpkg issues

## 📚 Advanced Usage

### Custom Build Flags
Edit `builder.cpp` to add custom compilation flags:
```cpp
compile_flags += " -march=native -flto";  // Add native optimizations
```

### Multiple Executables
Modify the build system to create multiple executables from different main files.

### Cross-Platform Support
The build system can be extended for Windows/macOS by adding platform-specific flags.

## 🎓 Learning Resources

- **Modern C++**: Use the template headers as examples of C++23 features
- **vcpkg**: Official Microsoft C++ package manager
- **Build Systems**: Understanding command-line compilation
- **VSCode C++**: Extension marketplace for additional tools

## 💡 Tips & Best Practices

### Code Organization
- Keep headers small and focused
- Use namespace organization
- Follow the snake_case/PascalCase convention established

### Build Management
- Use debug builds during development
- Profile with release builds
- Test both static and dynamic linking

### Package Management
- Pin package versions in vcpkg.json for reproducibility
- Use feature flags to control optional dependencies
- Keep vcpkg.json minimal (only direct dependencies)

---

## 🤝 Contributing to init++

To enhance the init++ tool itself:

1. **Add new template headers** to `templates/`
2. **Modify project structure** in `create_directory_structure()`
3. **Extend build system** in `create_build_system()`
4. **Update documentation** in `create_readme()`

The tool is designed to be easily extensible for your specific C++ development needs.
