# Template Setup Instructions

This directory should contain the template files that init++ uses to create new projects.

## Required Setup

### 1. vcpkg Template
The `templates/vcpkg/` directory should contain a vcpkg installation. To set it up:

```bash
# Clone vcpkg into the templates directory
git clone https://github.com/Microsoft/vcpkg.git templates/vcpkg

# Bootstrap vcpkg (optional - init++ will do this automatically)
cd templates/vcpkg
./bootstrap-vcpkg.sh
```

### 2. Template Headers
The `templates/include/` directory contains the template header files that are copied to new projects:
- `asyncops.hpp` - Async operations and coroutines utilities
- `raiiiofsw.hpp` - RAII filesystem wrappers  
- `stringformers.hpp` - String formatting and manipulation utilities
- `utilities.hpp` - General utility functions

## Directory Structure

After setup, your templates directory should look like:

```
templates/
├── include/
│   ├── asyncops.hpp
│   ├── raiiiofsw.hpp
│   ├── stringformers.hpp
│   └── utilities.hpp
└── vcpkg/              # (git clone required)
    ├── bootstrap-vcpkg.sh
    ├── vcpkg
    └── ... (vcpkg files)
```

## Notes

- The vcpkg directory is excluded from git (.gitignore) because it's a large repository
- Users need to clone vcpkg themselves as shown above
- The init++ tool will automatically bootstrap vcpkg when creating new projects
- Each generated project gets its own copy of vcpkg for isolation
