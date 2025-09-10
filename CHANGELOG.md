# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-09-10

### Added
- Initial release of init++ C++ project initializer
- Modern C++23 project template generation
- Custom command-line build system (no CMake required)
- vcpkg package manager integration with template copying
- Template headers included in every project:
  - `asyncops.hpp` - Async operations and coroutines
  - `raiiiofsw.hpp` - RAII filesystem wrappers
  - `stringformers.hpp` - String formatting utilities
  - `utilities.hpp` - General utility functions
- VSCode configuration with IntelliSense and build tasks
- Support for multiple build targets:
  - Static executables (default)
  - Static libraries (.a)
  - Dynamic libraries (.so)
- Debug and release build configurations
- Comprehensive documentation (README.md, QUICKSTART.md)
- Example project structure with tests
- Automatic dependency detection and compilation
- Cross-platform compatible code style and conventions

### Features
- Fast project initialization (seconds vs minutes)
- No external dependencies required (beyond standard build tools)
- Template-based approach for consistent project structure
- Pythonic naming conventions (PascalCase classes, snake_case methods)
- Allman indentation style formatting
- MIT license for open source compatibility

### Documentation
- Complete quick start guide with examples
- Troubleshooting section for common issues
- Advanced usage instructions for customization
- Contributing guidelines for community development
- Performance tips and best practices
