# Changelog

All notable changes to this project will be documented in this file.

## [1.1.0] - 2025-09-10

### Added
- **Full vcpkg Automation**: Complete automation of vcpkg management
  - Automatic baseline detection and updating
  - Binary caching setup (`.vcpkg-cache/` directory)
  - Automated package installation script (`install-packages.sh`)
  - Automated vcpkg update script (`update-vcpkg.sh`)
  - vcpkg configuration file generation (`vcpkg-configuration.json`)
- Enhanced vcpkg.json with schema validation and automatic baseline
- Executable scripts for easy package management
- Binary caching for faster package installation and rebuilds
- Comprehensive update documentation (`UPDATE_SUMMARY.md`)

### Changed
- **Simplified Package Management**: No more manual vcpkg commands
  - Use `./install-packages.sh` instead of `./vcpkg/vcpkg install`
  - Use `./update-vcpkg.sh` for vcpkg updates
- Removed all short command-line flags from build system
- Enhanced project structure with additional automation files
- Updated documentation to reflect fully automated approach

### Fixed
- Added missing `#include <cstdio>` for `popen` function
- Corrected all documentation inconsistencies
- Fixed template directory structure references
- Fixed vcpkg path references throughout documentation

### Benefits
- **Zero vcpkg Learning Curve**: No need to understand baselines, binary caching, or vcpkg internals
- **Faster Builds**: Automatic binary caching speeds up package installation
- **Easier Updates**: One command updates everything
- **Project Isolation**: Each project has its own vcpkg and cache
- **Future-Proof**: Automatic baseline management keeps packages up-to-date

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.1] - 2025-09-10

### Fixed
- Fixed command-line argument conflict between `--debug, -d` and `--dynamic, -d`
- Changed dynamic library option to `--dynamic, --dy` to avoid conflicts
- Added missing `#include <cstdlib>` to generated main.cpp template
- Updated documentation to match actual implementation:
  - Corrected template directory structure (`templates/include/` and `templates/vcpkg/`)
  - Fixed vcpkg path references (using `vcpkg/` not `.vcpkg/`)
  - Removed references to non-existent features (test framework, generated project headers)
  - Updated build system examples with correct command-line arguments

### Changed
- Documentation now accurately reflects the actual implementation
- QUICKSTART.md updated with correct directory structure and commands
- README.md updated with proper feature descriptions and examples

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
- Example project structure with tests directory
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
