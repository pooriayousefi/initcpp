# initcpp Quick Start Guide

A self-contained, modern C++ project initializer that creates well-structured C++ projects with embedded utility headers and a command-line build system.

## 🚀 Quick Start

### 1. Build the Initializer
```bash
cd /path/to/initcpp
g++ -std=c++23 main.cpp -o initcpp
```

### 2. Create a New Project
```bash
./initcpp /path/to/your-new-project
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

The `initcpp` tool creates a complete C++ project structure with:

- **Modern C++23 support**
- **Self-contained utility headers** (embedded in executable: asyncops, RAII filesystem wrappers, string formatters, utilities)
- **Command-line build system** (no CMake/Makefile needed)
- **VSCode configuration** (IntelliSense, tasks, formatting)
- **Multiple build targets** (executable, static lib, dynamic lib)
- **Basic project structure** with organized directories

## 🏗️ Project Structure Created

```
your-project/
├── include/                    # Header files (embedded content)
│   └── core/                  # Core template headers
│       ├── asyncops.hpp       # Async operations & coroutines
│       ├── raiiiofsw.hpp      # RAII filesystem wrappers
│       ├── stringformers.hpp # String formatting utilities
│       └── utilities.hpp     # General utility functions
├── src/                       # Source files
│   └── main.cpp               # Main entry point with basic template
├── tests/                     # Test directory (empty, ready for use)
├── build/                     # Build outputs
│   ├── debug/                 # Debug builds
│   └── release/               # Release builds
├── .vscode/                   # VSCode configuration
│   ├── settings.json          # C++ IntelliSense settings
│   └── tasks.json             # Build tasks
├── builder.cpp                # Build system source code
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
./initcpp ~/projects/my-awesome-app

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

## 🎯 Use Cases

### 1. Rapid Prototyping
```bash
./initcpp ~/prototypes/idea-test
cd ~/prototypes/idea-test
# Edit src/main.cpp with your idea
./builder --debug --executable && ./build/debug/idea_test
```

### 2. Library Development
```bash
./initcpp ~/libraries/my-lib
cd ~/libraries/my-lib
# Develop in src/, export in include/
./builder --release --static  # Creates lib*.a
```

### 3. Application Development
```bash
./initcpp ~/apps/my-app
cd ~/apps/my-app
# Build and run your application
./builder --release --executable
```

## ⚡ Performance Tips

### Fast Builds
- Use `--debug` for development (faster compilation)
- Use `--release` for final builds (optimized)
- The build system uses `-std=c++23` with modern optimizations

## 🔍 Troubleshooting

### Common Issues

**"Build errors"**
- Ensure GCC 11+ or Clang 14+ with C++23 support
- Check that all source files have proper syntax
- Verify include paths in build system

**"initcpp command not found"**
- Ensure you've built the executable: `g++ -std=c++23 main.cpp -o initcpp`
- Use `./initcpp` if not in PATH

**"Permission denied"**
- Make executable: `chmod +x initcpp`

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

### C++23 Features in Templates
The embedded headers showcase modern C++23 features:
- **Coroutines**: `asyncops.hpp` demonstrates co_await, co_yield
- **Ranges**: Modern STL ranges and algorithms
- **Concepts**: Type constraints and SFINAE replacement
- **Modules**: Ready for C++23 module adoption

### Build Systems
- Understanding command-line compilation
- Custom build system implementation
- Multi-target compilation strategies

### VSCode C++
- Extension marketplace for additional tools
- IntelliSense configuration
- Task automation

## 💡 Tips & Best Practices

### Code Organization
- Keep headers small and focused
- Use namespace organization
- Follow the snake_case/PascalCase convention established

### Build Management
- Use debug builds during development
- Profile with release builds
- Test both static and dynamic linking

---

## 🤝 Contributing to initcpp

To enhance the initcpp tool itself:

1. **Modify embedded headers** in `main.cpp` string literals
2. **Update project structure** in `create_directory_structure()`
3. **Extend build system** in `create_build_system()`
4. **Update documentation** in `create_readme()`
5. **Add new features** to the project generator

The tool is completely self-contained - all templates are embedded in the source code for maximum portability.
