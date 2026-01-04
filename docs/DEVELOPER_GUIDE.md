# filez Developer Guide

This guide covers everything you need to contribute to the filez project.

---

## Prerequisites

### Required

| Tool | Version | Notes |
|------|---------|-------|
| **C++20 Compiler** | MSVC 2019+, GCC 10+, Clang 12+ | Must support `std::filesystem`, `std::optional`, `std::chrono` |
| **CMake** | 3.16+ | Build system generator |
| **Git** | Latest | For submodules in `libs/` |

### Optional (for advanced providers)

| Tool | Purpose |
|------|---------|
| **vcpkg** | Package manager for Exiv2, BLAKE3, Tesseract, OpenCV, ONNX Runtime |
| **Ninja** | Faster builds than Make/MSBuild |
| **Google Test** | Unit testing (via vcpkg or system package) |

---

## Building from Source

### Quick Build (Windows)

```powershell
# Use the build script
build.bat

# Or manual build
cmake -S . -B build -G Ninja
cmake --build build
```

### Windows (MSVC)

```powershell
# Option 1: Visual Studio generator
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

# Option 2: Ninja with MSVC
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run
.\build\cli\fo_cli.exe --help
```

### Linux (GCC/Clang)

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install build-essential cmake libsqlite3-dev

# Optional: Install Google Test
sudo apt install libgtest-dev

# Build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run
./build/cli/fo_cli --help
```

### macOS (Clang)

```bash
# Install Xcode command-line tools
xcode-select --install

# Install dependencies via Homebrew
brew install cmake ninja

# Build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run
./build/cli/fo_cli --help
```

### vcpkg Integration

```bash
# Clone vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.sh  # or .bat on Windows

# Install optional dependencies
./vcpkg install gtest exiv2 blake3 tesseract opencv4 onnxruntime yaml-cpp

# Build with vcpkg toolchain
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `FO_BUILD_CLI` | ON | Build CLI executable |
| `FO_BUILD_GUI` | OFF | Build Qt GUI (legacy) |
| `FO_BUILD_BENCH` | ON | Build benchmarking tools |
| `FO_BUILD_TESTS` | ON | Build unit tests |

```bash
cmake -S . -B build -DFO_BUILD_TESTS=ON -DFO_BUILD_GUI=OFF
```

---

## Project Architecture

```
filez/
├── core/                    # fo_core static library
│   ├── include/fo/core/     # Public headers
│   │   ├── interfaces.hpp   # Provider interfaces (IFileScanner, IHasher, etc.)
│   │   ├── registry.hpp     # Template registry for providers
│   │   ├── types.hpp        # Common types (FileInfo, ImageMetadata)
│   │   └── ...
│   └── src/                 # Implementation
│       ├── scanner_std.cpp  # std::filesystem scanner
│       ├── scanner_win32.cpp # Win32 FindFirstFile scanner
│       ├── hasher_xxhash.cpp # xxHash hasher
│       ├── hasher_sha256.cpp # SHA-256 hasher
│       └── ...
├── cli/                     # fo_cli executable
├── gui/                     # Qt GUI (optional, legacy)
├── libs/                    # Vendored libraries (100+ submodules)
│   ├── xxHash/              # Fast hashing
│   ├── TinyEXIF/            # EXIF metadata
│   ├── sqlite3/             # Database
│   └── ...
├── tests/                   # Unit tests (Google Test)
├── benchmarks/              # Performance benchmarks
└── docs/                    # Documentation
```

### Key Components

| Component | Library/Executable | Description |
|-----------|-------------------|-------------|
| `fo_core` | Static library | Core engine with interfaces, providers, database |
| `fo_cli` | Executable | Command-line interface |
| `fo_tests` | Executable | Unit test runner |

### Provider Interfaces

All providers implement pure virtual interfaces defined in [`core/include/fo/core/interfaces.hpp`](../core/include/fo/core/interfaces.hpp):

```cpp
// File scanner interface
class IFileScanner {
public:
    virtual ~IFileScanner() = default;
    virtual std::string name() const = 0;
    virtual std::vector<FileInfo> scan(
        const std::vector<std::filesystem::path>& roots,
        const std::vector<std::string>& include_exts,
        bool follow_symlinks) = 0;
};

// Hasher interface
class IHasher {
public:
    virtual ~IHasher() = default;
    virtual std::string name() const = 0;
    virtual std::string fast64(const std::filesystem::path& p) = 0;
    virtual std::optional<std::string> strong(const std::filesystem::path& p) { return std::nullopt; }
    virtual std::string strong_algo() const { return ""; }
};

// Metadata provider interface
class IMetadataProvider {
public:
    virtual ~IMetadataProvider() = default;
    virtual std::string name() const = 0;
    virtual bool read(const std::filesystem::path& p, ImageMetadata& out) = 0;
    virtual bool write(const std::filesystem::path& p, const ImageMetadata& in) { return false; }
};

// Duplicate finder interface
class IDuplicateFinder {
public:
    virtual ~IDuplicateFinder() = default;
    virtual std::string name() const = 0;
    virtual std::vector<DuplicateGroup> group(const std::vector<FileInfo>& files, IHasher& hasher) = 0;
};
```

### Registry Pattern

Providers are registered and created via a singleton template registry ([`core/include/fo/core/registry.hpp`](../core/include/fo/core/registry.hpp)):

```cpp
template <typename T>
class Registry {
public:
    using Factory = std::function<std::unique_ptr<T>()>;

    static Registry& instance();           // Singleton access
    void add(const std::string& name, Factory f);   // Register a provider
    std::unique_ptr<T> create(const std::string& name) const;  // Create by name
    std::vector<std::string> names() const;  // List all registered names
};
```

**Usage:**
```cpp
// Create a hasher by name
auto hasher = Registry<IHasher>::instance().create("xxhash");

// List all available scanners
auto scanner_names = Registry<IFileScanner>::instance().names();
```

---

## Adding a New Provider

### Step 1: Create the Implementation File

Create a new `.cpp` file in `core/src/`. Follow the naming convention:
- Scanners: `scanner_<name>.cpp`
- Hashers: `hasher_<name>.cpp`
- Metadata: `metadata_<name>.cpp`

### Step 2: Implement the Interface

Example: Adding a new hasher (`core/src/hasher_myhasher.cpp`):

```cpp
#include "fo/core/interfaces.hpp"
#include "fo/core/registry.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>

namespace fo::core {

class MyHasher : public IHasher {
public:
    std::string name() const override { return "myhasher"; }

    std::string fast64(const std::filesystem::path& p) override {
        std::ifstream f(p, std::ios::binary);
        if (!f) return {};

        // Your hashing implementation here
        uint64_t hash = 0;
        // ... compute hash ...

        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << std::setw(16) << hash;
        return oss.str();
    }

    // Optional: implement strong() for cryptographic hash
    std::optional<std::string> strong(const std::filesystem::path& p) override {
        // Return std::nullopt if not supported
        return std::nullopt;
    }

    std::string strong_algo() const override { return ""; }
};

// Static registration - runs at program startup
static bool reg_hasher_myhasher = [](){
    Registry<IHasher>::instance().add("myhasher", [](){ 
        return std::make_unique<MyHasher>(); 
    });
    return true;
}();

// Force linkage function (optional, prevents dead code elimination)
void register_hasher_myhasher() { (void)reg_hasher_myhasher; }

} // namespace fo::core
```

### Step 3: Handle Optional Dependencies

If your provider requires external libraries, use compile-time guards:

```cpp
#ifdef FO_HAVE_MYLIB
#include <mylib/mylib.h>

namespace fo::core {

class MyLibHasher : public IHasher {
    // ... implementation using mylib ...
};

static bool reg = [](){
    Registry<IHasher>::instance().add("mylib", [](){ 
        return std::make_unique<MyLibHasher>(); 
    });
    return true;
}();

} // namespace fo::core

#endif // FO_HAVE_MYLIB
```

Update `core/CMakeLists.txt`:

```cmake
find_package(mylib CONFIG QUIET)

if(mylib_FOUND)
    message(STATUS "Found mylib, enabling mylib provider")
    target_compile_definitions(fo_core PRIVATE FO_HAVE_MYLIB)
    target_link_libraries(fo_core PUBLIC mylib::mylib)
endif()
```

### Step 4: Add to Provider Registration

If using explicit registration, add to `core/include/fo/core/provider_registration.hpp`:

```cpp
void register_hasher_myhasher();

inline void register_all_providers() {
    // ... existing registrations ...
    register_hasher_myhasher();
}
```

### Step 5: Write Tests

Add tests in `tests/test_myhasher.cpp`:

```cpp
#include <gtest/gtest.h>
#include "fo/core/registry.hpp"
#include "fo/core/interfaces.hpp"
#include "fo/core/provider_registration.hpp"
#include <fstream>
#include <filesystem>

using namespace fo::core;

class MyHasherTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        
        test_file = std::filesystem::temp_directory_path() / "fo_test_myhasher.txt";
        std::ofstream ofs(test_file);
        ofs << "Test content for hashing";
        ofs.close();
    }
    
    void TearDown() override {
        if (std::filesystem::exists(test_file)) {
            std::filesystem::remove(test_file);
        }
    }
    
    std::filesystem::path test_file;
};

TEST_F(MyHasherTest, HasherExists) {
    auto hasher = Registry<IHasher>::instance().create("myhasher");
    ASSERT_NE(hasher, nullptr);
    EXPECT_EQ(hasher->name(), "myhasher");
}

TEST_F(MyHasherTest, ProducesConsistentHash) {
    auto hasher = Registry<IHasher>::instance().create("myhasher");
    ASSERT_NE(hasher, nullptr);
    
    std::string hash1 = hasher->fast64(test_file);
    std::string hash2 = hasher->fast64(test_file);
    
    EXPECT_FALSE(hash1.empty());
    EXPECT_EQ(hash1, hash2);
}
```

Update `tests/CMakeLists.txt`:

```cmake
add_executable(fo_tests
    # ... existing tests ...
    test_myhasher.cpp
)
```

---

## Testing

### Running Tests

```powershell
# Build with tests enabled
cmake -S . -B build -DFO_BUILD_TESTS=ON
cmake --build build

# Run all tests
.\build\tests\fo_tests.exe

# Run specific test(s) by filter
.\build\tests\fo_tests.exe --gtest_filter=*Hasher*
.\build\tests\fo_tests.exe --gtest_filter=MyHasherTest.*

# Verbose output
.\build\tests\fo_tests.exe --gtest_output=xml:test_results.xml
```

### Test Structure

Tests use Google Test framework. Each test file follows this pattern:

```cpp
#include <gtest/gtest.h>
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"

using namespace fo::core;

class ProviderTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        // Create temporary test fixtures
    }
    
    void TearDown() override {
        // Clean up test fixtures
    }
};

TEST_F(ProviderTest, TestName) {
    // Arrange
    // Act
    // Assert
    EXPECT_TRUE(condition);
}
```

### Test Best Practices

1. **Use fixtures** for setup/teardown of temporary files
2. **Test edge cases**: empty files, missing files, large files
3. **Test consistency**: same input → same output
4. **Clean up**: always remove temp files in TearDown

---

## Fuzz Testing

This project uses LLVM's LibFuzzer-style structure for fuzz testing. We currently support "corpus-replay" mode where the fuzz targets are compiled as standard executables that can accept files as input or run a default dummy case.

### Targets
1. **`fuzz_metadata`**: Tests the `IMetadataProvider` implementations (e.g., TinyEXIF). It creates a temporary file from the input data and attempts to read metadata from it.
2. **`fuzz_rule_engine`**: Tests the `RuleEngine` by feeding random strings as rule configurations or file names.

### Running Fuzz Tests

To run the fuzz tests (sanity check):

```bash
# Run metadata fuzzer with internal dummy data
./build/tests/fuzz_metadata.exe

# Run rule engine fuzzer
./build/tests/fuzz_rule_engine.exe
```

### Running with a Corpus

To test against a specific file (simulating a crash case or seed):

```bash
./build/tests/fuzz_metadata.exe path/to/suspicious_image.jpg
```

*Note: For full coverage-guided fuzzing, these targets should be compiled with Clang and `-fsanitize=fuzzer` on Linux/macOS. On Windows MSVC, we currently use them for regression testing and stability checks.*

---

## Code Style

### C++20 Features

Use modern C++ idioms:

```cpp
// Filesystem
#include <filesystem>
namespace fs = std::filesystem;

// Optional return values
std::optional<std::string> find_value(const std::string& key);

// Chrono for time
#include <chrono>
using Clock = std::chrono::system_clock;

// Structured bindings
for (const auto& [key, value] : map) { }

// Range-based algorithms (C++20)
std::ranges::sort(container);
```

### Naming Conventions

| Element | Style | Example |
|---------|-------|---------|
| Classes | `CamelCase` | `StdFsScanner`, `XXHasher` |
| Functions | `snake_case` | `find_duplicates()`, `read_metadata()` |
| Variables | `snake_case` | `file_count`, `include_exts` |
| Constants | `UPPER_SNAKE` | `MAX_BUFFER_SIZE` |
| Namespaces | `snake_case` | `fo::core` |
| Files | `snake_case` | `scanner_std.cpp`, `hasher_xxhash.cpp` |

### Header Files

```cpp
#pragma once  // Use pragma once, not include guards

// Group includes: std → external → internal
#include <string>
#include <vector>
#include <filesystem>

#include <external/library.h>

#include "fo/core/types.hpp"
#include "fo/core/interfaces.hpp"

namespace fo::core {
// ...
} // namespace fo::core
```

### Platform-Specific Code

```cpp
// Guard platform-specific code
#ifdef _WIN32
    // Windows-specific implementation
    #include <windows.h>
#else
    // POSIX implementation
    #include <dirent.h>
#endif

// Optional dependency guards
#ifdef FO_HAVE_TESSERACT
    #include <tesseract/baseapi.h>
    // Tesseract-specific code
#endif
```

### Error Handling

```cpp
// Use std::optional for expected missing values
std::optional<FileInfo> find_file(const std::string& name);

// Use std::error_code for filesystem operations
std::error_code ec;
auto size = fs::file_size(path, ec);
if (ec) {
    // Handle error
}

// Avoid exceptions in hot paths
```

---

## Contributing

### Branch Workflow

1. **Fork** the repository on GitHub
2. **Create a feature branch** from `main`:
   ```bash
   git checkout -b feat/my-new-feature
   ```
3. **Make changes** following code style guidelines
4. **Run tests** and ensure they pass
5. **Commit** with conventional commit messages
6. **Push** and open a Pull Request

### Commit Message Format

Use [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>: <description>

[optional body]

[optional footer]
```

**Types:**
| Type | Description |
|------|-------------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation changes |
| `refactor` | Code refactoring (no feature/fix) |
| `test` | Adding or updating tests |
| `chore` | Build, tooling, or maintenance |

**Examples:**
```
feat: add BLAKE3 hasher provider

fix: handle empty files in xxHash

docs: add developer guide

refactor: extract common hash formatting to utility

test: add consistency tests for all hashers

chore: update vcpkg baseline
```

### Pull Request Process

1. **Title**: Use conventional commit format
2. **Description**: Explain what and why
3. **Tests**: All tests must pass
4. **Review**: Address feedback promptly
5. **Squash**: Keep history clean (squash if needed)

### Code Review Checklist

- [ ] Follows code style conventions
- [ ] Tests added/updated for new functionality
- [ ] No compiler warnings
- [ ] Documentation updated if needed
- [ ] No secrets or keys committed
- [ ] Platform guards for OS-specific code
- [ ] Optional dependency guards (`FO_HAVE_*`)

---

## Troubleshooting

### Build Issues

**CMake can't find vcpkg packages:**
```bash
# Ensure VCPKG_ROOT is set, or specify toolchain explicitly
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

**Linker errors for providers:**
```cpp
// Ensure registration function is called to prevent dead code elimination
void register_hasher_myhasher() { (void)reg_hasher_myhasher; }
```

**Tests not discovering providers:**
```cpp
// Call register_all_providers() in SetUp()
void SetUp() override {
    register_all_providers();
}
```

### Runtime Issues

**Provider not found:**
```cpp
auto hasher = Registry<IHasher>::instance().create("unknown");
// Returns nullptr - always check!
if (!hasher) {
    // Handle missing provider
}
```

**List available providers:**
```cpp
for (const auto& name : Registry<IHasher>::instance().names()) {
    std::cout << "Available hasher: " << name << "\n";
}
```

---

## Resources

- [README.md](../README.md) - Project overview
- [README_CLI.md](../README_CLI.md) - CLI usage guide
- [AGENTS.md](../AGENTS.md) - Quick reference for AI agents
- [docs/LIBRARY_EVALUATION.md](LIBRARY_EVALUATION.md) - Provider benchmarks
- [docs/DATABASE_SCHEMA.md](DATABASE_SCHEMA.md) - SQLite schema
- [docs/ROADMAP.md](ROADMAP.md) - Development roadmap
