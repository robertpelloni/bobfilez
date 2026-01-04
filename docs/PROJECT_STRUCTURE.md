# Project Structure

This document outlines the directory layout of the filez project.

## Top-Level Directories

| Directory | Description |
|-----------|-------------|
| `core/` | The core C++ static library (`fo_core`). Contains all business logic, database access, and providers. |
| `cli/` | The command-line interface executable (`fo_cli`). Links against `fo_core`. |
| `gui/` | The Qt6-based graphical user interface (`fo_gui`). Links against `fo_core`. |
| `libs/` | Third-party dependencies managed as Git submodules (130+ libraries). |
| `tests/` | Unit and integration tests using Google Test. |
| `benchmarks/` | Performance benchmarks using Google Benchmark. |
| `docs/` | Project documentation, including roadmap, API reference, and user manual. |
| `scripts/` | Helper scripts for building, packaging, and maintenance. |
| `examples/` | Example configuration files (e.g., `rules.yaml`). |

## Core Library (`core/`)

The `core/` directory is the heart of the application. It is designed to be portable and UI-agnostic.

### `core/include/fo/core/` (Public Headers)
- **`interfaces.hpp`**: Defines abstract base classes for providers (`IFileScanner`, `IHasher`, `IMetadataProvider`).
- **`types.hpp`**: Common data structures (`FileInfo`, `DateMetadata`).
- **`registry.hpp`**: The `Registry<T>` template for dependency injection.
- **`engine.hpp`**: The main `Engine` class that orchestrates operations.
- **`database.hpp`**: Database connection and migration management.
- **`rule_engine.hpp`**: Logic for smart organization rules.

### `core/src/` (Implementation)
- **`providers/`**: Concrete implementations of interfaces (e.g., `scanner_win32.cpp`, `hasher_blake3.cpp`).
- **`database.cpp`**: SQLite implementation.
- **`engine.cpp`**: Core logic implementation.

## CLI (`cli/`)

The CLI is a thin wrapper around the Core library.
- **`fo_cli.cpp`**: Main entry point. Parses arguments using `CLI11` (or similar) and calls `Engine` methods.

## GUI (`gui/`)

The GUI is a Qt Widgets application.
- **`mainwindow.cpp`**: The main application window.
- **`models/`**: Qt AbstractItemModel implementations for displaying database results.

## Libraries (`libs/`)

We vendor many dependencies to ensure reproducibility and offline builds. See `docs/SUBMODULES.md` for a full list.
- **`TinyEXIF`**: Lightweight EXIF parsing.
- **`hash-library`**: Portable hash implementations (SHA256, MD5).
- **`xxHash`**: Extremely fast non-cryptographic hash.
- **`sqlite3`**: The database engine.

## Build System

The project uses CMake.
- **`CMakeLists.txt`**: Top-level build configuration.
- **`vcpkg.json`**: Manifest for dependencies managed by vcpkg (e.g., Exiv2, Tesseract).
