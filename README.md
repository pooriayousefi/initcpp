# initcpp 🚀

A self-contained, modern C++ project initializer that creates well-structured C++ projects with embedded utility headers and a command-line build system.

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](README.md)

## ✨ Features

- **🏃‍♂️ Fast Setup**: Create complete C++ projects in seconds
- **📦 Self-Contained**: All template headers embedded - no external dependencies
- **🎯 Modern C++23**: Full support for the latest C++ standard
- **🔧 No CMake Required**: Custom command-line build system
- **📚 Rich Templates**: Pre-built utility headers in `include/core/`
- **🎨 VSCode Ready**: Complete IDE configuration
- **📱 Multiple Targets**: Executables, static libs, dynamic libs

## 🚀 Quick Start

```bash
# Build the initializer
g++ -std=c++23 main.cpp -o initcpp

# Create a new project
./initcpp ~/my-awesome-project

# Build and run
cd ~/my-awesome-project
g++ -std=c++23 builder.cpp -o builder
./builder --release --executable
./build/release/my_awesome_project
```

## 📋 What Gets Created

Every project includes:

- **Embedded Headers**: Core template headers in `include/core/` (written directly from embedded content)
- **Build System**: Command-line builder with multiple configurations
- **IDE Support**: Full VSCode configuration with IntelliSense
- **Project Structure**: Organized `src/`, `include/core/`, `tests/`, `build/` directories
- **Documentation**: Complete README with usage instructions

## 🏗️ Generated Project Structure

```
your-project/
├── include/                   # Header files (embedded content)
│   └── core/                  # Core template headers
│       ├── asyncops.hpp       # Async operations & coroutines
│       ├── raiiiofsw.hpp      # RAII filesystem wrappers
│       ├── stringformers.hpp  # String formatting utilities
│       └── utilities.hpp      # General utility functions
├── src/                       # Source files
│   └── main.cpp               # Main entry point with basic template
├── tests/                     # Test directory (empty, ready for use)
├── build/                     # Build outputs
│   ├── debug/                 # Debug builds
│   └── release/               # Release builds
├── .vscode/                   # VSCode configuration
│   ├── settings.json          # C++ IntelliSense settings
│   └── tasks.json             # Build tasks
├── builder.cpp                # Custom build system source
└── README.md                  # Project documentation
```

## 🎯 Use Cases

### 🔬 Rapid Prototyping
Perfect for testing ideas quickly with modern C++ features.

### 📚 Library Development  
Build static/dynamic libraries with proper packaging and documentation.

### 🖥️ Application Development
Full-featured applications with build systems.

### 🎓 Learning & Teaching
Great starting point for C++ education with modern best practices.

## 🔧 Build System Usage

The generated build system supports multiple configurations:

### Build Types
- `--debug`: Debug build with symbols and no optimization
- `--release`: Release build with optimizations

### Output Types  
- `--executable`: Static executable (default)
- `--static`: Static library (.a)
- `--dynamic`: Dynamic library (.so)

### Examples
```bash
# Debug executable
./builder --debug --executable

# Release static library
./builder --release --static

# Release dynamic library  
./builder --release --dynamic
```

## 📦 Template Headers Included

### `asyncops.hpp`
- Coroutines and async operations
- Generator utilities  
- Modern async patterns

### `raiiiofsw.hpp`
- RAII filesystem wrappers
- Safe file operations
- Automatic resource management

### `stringformers.hpp`
- String formatting utilities
- Type-safe string operations
- Performance optimizations

### `utilities.hpp`
- General-purpose functions
- Cross-platform compatibility
- Common algorithms and helpers

## 📚 Documentation

- **[📖 Quick Start Guide](QUICKSTART.md)** - Complete usage documentation
- **🎯 Project Templates** - See generated project README files
- **🔧 Build System** - Command-line builder documentation

## 🛠️ Requirements

- **Compiler**: GCC 11+ or Clang 14+ with C++23 support
- **Platform**: Linux x64 (Ubuntu/Debian tested)
- **Dependencies**: Standard build tools (no external dependencies)

## 📦 Installation & Distribution

### Building from Source
```bash
git clone https://github.com/pooriayousefi/initcpp.git
cd initcpp
g++ -std=c++23 -O3 main.cpp -o initcpp
```

### Distributing the Tool
The `initcpp` executable is completely self-contained:
- **No external files needed**: All template headers are embedded
- **Portable**: Copy the single executable anywhere
- **Zero setup**: Works immediately on any compatible system

```bash
# Copy to system PATH for global access
sudo cp initcpp /usr/local/bin/

# Or use directly
./initcpp /path/to/new-project
```

## 🎨 Code Style

Projects follow these conventions:
- **Classes/Structs**: PascalCase (`ExampleClass`)
- **Methods/Variables**: snake_case (`get_name()`, `project_name_`)
- **Indentation**: Allman style (braces on new lines)
- **Standard**: Modern C++23 features encouraged

## 🤝 Contributing

We welcome contributions! Here's how you can help:

1. **🐛 Bug Reports**: Found an issue? Open an issue with details
2. **💡 Feature Requests**: Have an idea? We'd love to hear it
3. **🔧 Pull Requests**: Fix bugs or add features
4. **📚 Documentation**: Help improve guides and examples
5. **🧪 Testing**: Test on different platforms and configurations

### Development Setup

```bash
git clone https://github.com/pooriayousefi/initcpp.git
cd initcpp
g++ -std=c++23 main.cpp -o initcpp
./initcpp test-project  # Test your changes
```

## 📈 Roadmap

- [ ] **Windows Support**: Cross-platform compatibility
- [ ] **macOS Support**: Apple Silicon and Intel support  
- [ ] **More Templates**: Additional project types and frameworks
- [ ] **Package Templates**: Pre-configured packages for common use cases
- [ ] **CI/CD Integration**: GitHub Actions and other CI systems
- [ ] **Plugin System**: Extensible template system

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **C++ Standards Committee** - For continuous language evolution
- **Open Source Community** - For inspiration and best practices

##  Support

- **📫 Issues**: [GitHub Issues](https://github.com/pooriayousefi/initcpp/issues)
- **💬 Discussions**: [GitHub Discussions](https://github.com/pooriayousefi/initcpp/discussions)
- **📧 Contact**: Direct contact via GitHub

---

**⭐ Star this repo if you find it useful!**

Made with ❤️ for the C++ community
