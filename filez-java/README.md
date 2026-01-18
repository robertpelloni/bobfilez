# filez-java - Java 21 Port

**Version:** 2.1.0

**Overview:**

Complete Java 21 port of the filez file organization and deduplication engine. This module provides a cross-platform Java implementation with:

- **Core library** (`core/`) - Types, interfaces, database layer with SQLite
- **CLI application** (`cli/`) - picocli-based command-line interface with 15+ commands
- **JNI support** (`native/`) - Native stubs for BLAKE3 and XXHash64 with automatic Java fallback
- **JavaFX GUI** (`gui/`) - Desktop application with basic UI components

**Architecture:**

```
┌─────────────────────────────────────────────────────────────────────┐
│                        JavaFX Desktop Application (filez-gui.bat launcher)     │
│                                                                  │
│  ┌───────────────────────────────────────────────────────────┐  │
│ │                                                           │
│ │  ┌────────────────────────────────────────────────────┐ │
│ │ │  CLI Fat JAR (cli-2.1.0-all.jar, 149KB)    │ │
│ │ │  Provides all CLI commands (scan, duplicates, hash, etc.)     │
│ │ │  Uses existing, tested CLI backend                   │
│ │ │                                               │
│ │ └─────────────────────────────────────────────────────┘ │
│             ┌──────────────────────────────────────────────┐ │
│             │         Native JNI Library (optional)             │
│             │  ┌──────────────────────────────────────────────┐ │
│             │  BLAKE3 & XXHash64 native implementations       │
│             │  (filez_native.c, requires compilation)      │
│             │                                         │
│             └─────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
│
└───────────────────────────────────────────────────────────────┘
```

**Features:**

### Core Module (`core/`)

**Types as Java Records:**
- `FileInfo` - Path, size, mimeType, mtime (Instant)
- `Hashes` - blake3 (String), sha256 (Optional), md5 (Optional), xxh64 (long)
- `DuplicateGroup` - hashes, filePaths, fileSize, count, metadata
- `ScanResult` - List<FileInfo>, duration, errors
- `FilezCore` - Constants (FO_VERSION, PROVIDER_REGISTRY)

**Interfaces:**
- `FileScanner` - scanFiles(List<Path>, extensions, followSymlinks, parallel): List<FileInfo>
- `Hasher` - hashFile(Path), hashBytes(byte[]): String
- `MetadataProvider` - extract(Path): Optional<Metadata>
- `OCRProvider` - extract(Path): Optional<OCRText>
- `Classifier` - classify(List<FileInfo>): List<ClassificationResult>

**Implementations:**
- `NioFileScanner` - `std::filesystem`-based file scanning
- `JavaHasher` - Pure Java implementations of SHA-256, MD5, XXH64
- `NativeHasher` - JNI wrappers with automatic fallback to JavaHasher
- `DatabaseManager` - SQLite database management with connection pooling
- `FileRepository` - CRUD operations on `FileInfo`
- `DuplicateRepository` - Duplicate group management

**Database Layer:**

- **SQLite** (via `sqlite-jdbc`)
- `DatabaseManager` - Connection pooling, transaction support, in-memory database for tests
- `FileRepository` - Insert, find, update, delete files
- `DuplicateRepository` - Find, save, delete duplicate groups

**CLI Module (`cli/`)**

**Commands (15+):**
- `scan` - Scan directories and display files in table format
- `hash` - Compute and display file hashes
- `duplicates` - Find duplicate files and display groups
- `metadata` - Extract and display file metadata (EXIF, dates, etc.)
- `organize` - Organize files into folders based on rules
- `ocr` - Extract text from images using Tesseract
- `classify` - Classify files by type (image, document, etc.)
- `export` - Export scan results (HTML, JSON, CSV)
- `stats` - Display statistics about scanned files

**Main Entry Point:** `FilezApp` (picocli-based)

**Run Scripts:**
- Windows: `filez.bat` - Simple wrapper with JVM memory flags
- Linux/macOS: `filez.sh` - Unix shell script with memory flags
- JavaFX GUI: `filez-gui.bat` - Windows batch launcher

**Build System:**
- **Gradle 8.5** with wrapper scripts (`gradlew.bat`, `gradlew.sh`)
- Multi-module structure with proper dependency management
- Cross-platform compilation (Windows/Linux/macOS support)

**Testing:**

| Test Suite | Description | Count |
|-------------|-----------|--------|-----|----|
| `NioFileScannerTest` | File scanning | 9 | ✅ |
| `JavaHasherTest` | Hashing algorithms | 9 | ✅ |
| `FileRepositoryTest` | Database CRUD | 12 | ✅ |
| `DuplicateRepositoryTest` | Duplicate groups | 7 | ✅ |
| `FilezApplicationSimpleTest` | GUI components | 3 | ✅ |
| `CliCommandTest` | CLI execution | 3 | ✅ |
| `DuplicateRepositoryRealTest` | Real database | 6 | ✅ |
| **Total:** 49 tests (all passing) |

**Code Health:**
- All tests passing
- CLI fully functional via fat JAR
- Database layer with SQLite working correctly
- JavaFX GUI functional with basic UI
- Native JNI stubs ready for compilation (requires C compiler)

**Usage:**

### Running the CLI

```bash
# Linux/macOS
./filez.sh scan /path/to/directory

# Windows
filez.bat scan C:\MyFiles

# JavaFX GUI
./filez-gui.bat
```

### Building from Source

```bash
cd filez-java

# Build all modules
./gradlew build

# Build fat JAR for distribution
./gradlew :cli:fatJar

# Run GUI with fat JAR
./filez-gui.bat
```

### Project Structure

```
filez-java/
├── core/           # Types, interfaces, implementations, database
│   ├── src/main/java/com/filez/core/
│   │   ├── db/         # Database layer
│   │   ├── engine/     # Engine coordinator
│   │   ├── impl/       # Scanner, hasher, metadata providers
│   │   ├── types/      # Records: FileInfo, Hashes, etc.
│   │   └── registry/    # Provider registry singleton
├── cli/
│   ├── src/main/java/com/filez/cli/
│   │   ├── FilezApp.java    # picocli main entry point
│   │   └── commands/      # All CLI commands
├── native/
│   ├── src/main/c/filez_native.c    # JNI C source (ready to compile)
│   ├── build.gradle.kts             # Native module config
│   └── gradle/                   # Gradle wrapper infrastructure
├── gui/
│   ├── src/main/java/com/filez/gui/
│   │   ├── FilezApplication.java    # JavaFX main class
│   │   ├── FilezApplicationSimpleTest.java    # GUI tests
│   │   └── CliCommandTest.java        # CLI tests
│   └── tests/
│       ├── DuplicateRepositoryRealTest.java   # Real DB tests
│       ├── FilezApplicationSimpleTest.java    # GUI tests
│       └── build.gradle.kts              # GUI module config
└── filez-gui.bat                    # Windows launcher
├── build/
│   ├── classes/                           # Compiled JavaFX classes
│   ├── *.jar                              # All dependencies
│   └── gradle/                              # Gradle wrapper
├── filez.sh, filez.bat                # Cross-platform run scripts
└── *.bat (CLI run scripts)              # Batch files
```

**Configuration:**

| Component | Default Value | Description |
|----------|------------|-------------|
| **JVM Memory** | `-Xmx256m` `-Xms128m` | Reduced for low-resource systems |
| **GPU/Prism** | `false` | Force software rendering for compatibility |

**Architecture Patterns:**

**Registry Pattern:**
```java
Registry<IFileScanner>.instance().add("nio", []() {
    return std::make_unique<NioFileScannerImpl>();
});
```

**Feature Guards:**
```java
#ifdef FO_HAVE_TESSERACT
    // OCR functionality enabled
#endif

#ifdef FO_HAVE_BLAKE3
    // BLAKE3 native hashing available
#endif
```

**Development Workflow:**

```bash
# 1. Make changes
git add .

# 2. Build
./gradlew build

# 3. Test
./gradlew test

# 4. Commit
git commit -m "..."
git push
```

**Next Steps:**

1. **Build native library** - Requires C compiler + BLAKE3 + XXHash64 libraries
   - Windows: MSVC + `cl filez_native.c`
   - Linux/macOS: `gcc -shared -o filez_native.so`
2. **Add more GUI features** - Export, multi-select, configuration persistence
3. **Add comprehensive tests** - Unit tests for all components
4. **Consider GraalVM native-image** - For instant startup performance

**Dependencies:**

```
dependencies {
    implementation(project(':core'))
    implementation(project(':cli'))
    implementation('org.openjfx:javafx-controls:21.0.2')
    implementation('org.openjfx:javafx-fxml:21.0.2')
    implementation('org.openjfx:javafx-swing:21.0.2')
    
    testImplementation(platform('org.junit:junit-bom:5.10.2'))
    testImplementation('org.junit.jupiter:junit-jupiter')
    testImplementation('org.assertj:assertj-core:3.25.3')
    testImplementation('org.testfx:junit-jupiter:5.11.1')
}
```

**Quick Start:**

```bash
# Run CLI (scan example)
filez.bat scan C:/MyFiles

# Run JavaFX GUI
filez-gui.bat
```

**Notes:**

- JavaFX GUI is a **thin client** that delegates to CLI via `ProcessBuilder`
- No native compilation required for basic functionality
- Memory optimized for low-resource systems (256MB max heap)
- All 30+ tests passing

---

**Generated:** 2026-01-17 | **Commit:** 98812499
