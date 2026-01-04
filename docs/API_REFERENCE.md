# filez API Reference

## Generating API Documentation

### Prerequisites
Install Doxygen from https://www.doxygen.nl/download.html

### Generate Documentation
```bash
# From project root
doxygen docs/Doxyfile

# Open the generated documentation
start docs/api/html/index.html  # Windows
open docs/api/html/index.html   # macOS
xdg-open docs/api/html/index.html  # Linux
```

The generated HTML documentation will be in `docs/api/html/`.

---

## Core API Overview

All core types are in the `fo::core` namespace.

### Interfaces

#### IFileScanner
Abstract interface for filesystem scanning implementations.

```cpp
class IFileScanner {
public:
    virtual std::string name() const = 0;
    virtual std::vector<FileInfo> scan(
        const std::vector<std::filesystem::path>& roots,
        const std::vector<std::string>& include_exts,
        bool follow_symlinks) = 0;
};
```

**Implementations:** `StdScanner` (uses `std::filesystem`)

---

#### IHasher
Abstract interface for file hashing implementations.

```cpp
class IHasher {
public:
    virtual std::string name() const = 0;
    virtual std::string fast64(const std::filesystem::path& p) = 0;
    virtual std::optional<std::string> strong(const std::filesystem::path& p);
    virtual std::string strong_algo() const;
};
```

**Methods:**
- `fast64()` - Quick 64-bit hash for initial duplicate detection
- `strong()` - Optional cryptographic hash for verification
- `strong_algo()` - Returns algorithm name (e.g., "SHA256")

**Implementations:** `XxHasher` (xxHash64)

---

#### IMetadataProvider
Abstract interface for reading/writing file metadata (EXIF, etc.).

```cpp
class IMetadataProvider {
public:
    virtual std::string name() const = 0;
    virtual bool read(const std::filesystem::path& p, ImageMetadata& out) = 0;
    virtual bool write(const std::filesystem::path& p, const ImageMetadata& in);
};
```

**Implementations:** `TinyExifProvider`, `ExifToolProvider`

---

#### IDuplicateFinder
Abstract interface for duplicate detection algorithms.

```cpp
class IDuplicateFinder {
public:
    virtual std::string name() const = 0;
    virtual std::vector<DuplicateGroup> group(
        const std::vector<FileInfo>& files, 
        IHasher& hasher) = 0;
};
```

---

### Engine Class

The `Engine` class is the main entry point for file organization operations.

```cpp
class Engine {
public:
    explicit Engine(EngineConfig cfg = {});
    
    // Scan directories for files
    std::vector<FileInfo> scan(
        const std::vector<std::filesystem::path>& roots,
        const std::vector<std::string>& include_exts,
        bool follow_symlinks,
        bool prune = false);
    
    // Find duplicate files
    std::vector<DuplicateGroup> find_duplicates(const std::vector<FileInfo>& files);
    
    // Repository accessors
    FileRepository& file_repository();
    DuplicateRepository& duplicate_repository();
    IgnoreRepository& ignore_repository();
    ScanSessionRepository& session_repository();
    DatabaseManager& database();
    IHasher& hasher();
};
```

**Configuration:**
```cpp
struct EngineConfig {
    std::string scanner = "std";      // Scanner implementation name
    std::string hasher = "fast64";    // Hasher implementation name
    std::string db_path = "fo.db";    // SQLite database path
    bool use_ads_cache = false;       // Use NTFS ADS for hash caching
};
```

---

### Registry Pattern

The `Registry<T>` template provides a plugin-like system for registering and creating interface implementations.

```cpp
template <typename T>
class Registry {
public:
    static Registry& instance();
    
    void add(const std::string& name, Factory f);
    std::unique_ptr<T> create(const std::string& name) const;
    std::vector<std::string> names() const;
};
```

**Usage:**
```cpp
// Register a scanner
Registry<IFileScanner>::instance().add("std", []() {
    return std::make_unique<StdScanner>();
});

// Create by name
auto scanner = Registry<IFileScanner>::instance().create("std");

// List available implementations
auto names = Registry<IFileScanner>::instance().names();
```

**Available Registries:**
- `Registry<IFileScanner>` - File scanners
- `Registry<IHasher>` - Hash algorithms
- `Registry<IMetadataProvider>` - Metadata readers/writers
- `Registry<IOcrProvider>` - OCR engines (optional)
- `Registry<IPerceptualHashProvider>` - Perceptual hashing (optional)

---

### Database Classes

#### DatabaseManager
Manages SQLite database connections and schema migrations.

```cpp
class DatabaseManager {
public:
    void open(const std::filesystem::path& db_path);
    void close();
    void migrate();
    sqlite3* get_db() const;
    void execute(const std::string& sql);
    int query_int(const std::string& sql);
};
```

#### Repository Classes
Type-safe data access layers built on `DatabaseManager`:

- **FileRepository** - CRUD for indexed files
- **DuplicateRepository** - Store/retrieve duplicate groups
- **IgnoreRepository** - Manage ignored paths
- **ScanSessionRepository** - Track scan history

---

### Core Types

```cpp
struct FileInfo {
    std::filesystem::path path;
    std::uintmax_t size;
    std::filesystem::file_time_type mtime;
    std::string fast_hash;
    std::optional<std::string> strong_hash;
};

struct ImageMetadata {
    std::optional<std::string> date_taken;
    std::optional<double> latitude;
    std::optional<double> longitude;
    std::optional<int> width;
    std::optional<int> height;
    // ... additional fields
};

struct DuplicateGroup {
    std::uintmax_t size;
    std::string fast64;
    std::vector<FileInfo> files;
};
```

---

## Example Usage

```cpp
#include <fo/core/engine.hpp>

int main() {
    fo::core::EngineConfig cfg;
    cfg.db_path = "my_index.db";
    
    fo::core::Engine engine(cfg);
    
    // Scan directories
    auto files = engine.scan(
        {"/photos", "/downloads"},
        {".jpg", ".png", ".gif"},
        false  // don't follow symlinks
    );
    
    // Find duplicates
    auto dupes = engine.find_duplicates(files);
    
    for (const auto& group : dupes) {
        std::cout << "Duplicate group (" << group.size << " bytes):\n";
        for (const auto& f : group.files) {
            std::cout << "  " << f.path << "\n";
        }
    }
    
    return 0;
}
```
