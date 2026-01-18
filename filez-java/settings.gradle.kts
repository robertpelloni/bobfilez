// Register all subprojects in the filez-java multi-module build
rootProject.name = "filez-java"

// Define all modules
include("core")      // Core types, interfaces, database layer
include("cli")       // Picocli-based CLI application
include("native")    // JNI wrappers for native code
include("gui")       // JavaFX desktop application
include("tests")     // Integration tests
