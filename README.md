# init++ 🚀

A modern C++ project initializer that creates well-structured C++ projects with custom utility headers, vcpkg integration, and a command-line build system.

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](README.md)

## ✨ Features

- **🏃‍♂️ Fast Setup**: Create complete C++ projects in seconds
- **🎯 Modern C++23**: Full support for the latest C++ standard
- **📦 vcpkg Integration**: Package management made simple
- **🔧 No CMake Required**: Custom command-line build system
- **📚 Rich Templates**: Pre-built utility headers included
- **🎨 VSCode Ready**: Complete IDE configuration
- **📱 Multiple Targets**: Executables, static libs, dynamic libs

## 🚀 Quick Start

```bash
# Build the initializer
g++ -std=c++23 main.cpp -o init++

# Create a new project
./init++ ~/my-awesome-project

# Build and run
cd ~/my-awesome-project
g++ -std=c++23 builder.cpp -o builder
./builder --release --executable
./build/release/my_awesome_project
```

## 📋 What Gets Created

Every project includes:

- **Custom Headers**: `asyncops.hpp`, `raiiiofsw.hpp`, `stringformers.hpp`, `utilities.hpp`
- **Build System**: Command-line builder with multiple configurations
- **Package Manager**: Local vcpkg installation for dependencies
- **IDE Support**: Full VSCode configuration with IntelliSense
- **Project Structure**: Organized `src/`, `include/`, `tests/`, `build/` directories
- **Documentation**: Complete README with usage instructions

## 🏗️ Generated Project Structure

```
your-project/
├── include/                    # Headers + utility templates
├── src/                        # Source files with examples
├── tests/                      # Test framework ready
├── build/debug/               # Debug builds
├── build/release/             # Optimized builds
├── .vcpkg/                    # Local package manager
├── .vscode/                   # IDE configuration
├── builder.cpp                # Custom build system
└── README.md                  # Project documentation
```

## 🎯 Use Cases

### 🔬 Rapid Prototyping
Perfect for testing ideas quickly with modern C++ features.

### 📚 Library Development  
Build static/dynamic libraries with proper packaging and documentation.

### 🖥️ Application Development
Full-featured applications with dependency management and build systems.

### 🎓 Learning & Teaching
Great starting point for C++ education with modern best practices.

## 🔧 Build System Features

The generated build system supports:

- **Debug/Release** configurations
- **Static executables** (default)
- **Static libraries** (.a files)
- **Dynamic libraries** (.so files)
- **Automatic dependency detection**
- **vcpkg integration**
- **Modern C++23 flags**

```bash
# Examples
./builder --debug --executable      # Debug executable
./builder --release --static        # Release static library
./builder --release --dynamic       # Release dynamic library
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
- **📦 Package Management** - vcpkg integration guide

## 🛠️ Requirements

- **Compiler**: GCC 11+ or Clang 14+ with C++23 support
- **Platform**: Linux x64 (Ubuntu/Debian tested)
- **Dependencies**: Standard build tools (no external dependencies)

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
git clone https://github.com/pooriayousefi/init++.git
cd init++
g++ -std=c++23 main.cpp -o init++
./init++ test-project  # Test your changes
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

- **vcpkg Team** - For the excellent package manager
- **C++ Standards Committee** - For continuous language evolution
- **Open Source Community** - For inspiration and best practices

## 📞 Support

- **📫 Issues**: [GitHub Issues](https://github.com/pooriayousefi/init++/issues)
- **💬 Discussions**: [GitHub Discussions](https://github.com/pooriayousefi/init++/discussions)
- **📧 Contact**: [Your Email] (for direct contact)

---

**⭐ Star this repo if you find it useful!**

Made with ❤️ for the C++ community
