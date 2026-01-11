plugins {
    `java-library`
}

dependencies {
    api(project(":core"))
    
    // Null safety annotations
    compileOnly("org.jetbrains:annotations:24.1.0")
    
    // Testing
    testImplementation(platform("org.junit:junit-bom:5.10.2"))
    testImplementation("org.junit.jupiter:junit-jupiter")
}

// JNI header generation task
tasks.register<Exec>("generateJniHeaders") {
    dependsOn("compileJava")
    
    val jniClasses = listOf(
        "com.filez.native.Blake3Hasher",
        "com.filez.native.NativeFileScanner",
        "com.filez.native.XxHasher"
    )
    
    val outputDir = file("src/main/cpp/include")
    outputDir.mkdirs()
    
    commandLine(
        "javac",
        "-h", outputDir.absolutePath,
        "-d", layout.buildDirectory.dir("classes/java/main").get().asFile.absolutePath,
        *jniClasses.map { 
            file("src/main/java/${it.replace('.', '/')}.java").absolutePath 
        }.toTypedArray()
    )
}

// Native library resource directories per platform
val nativeLibDir = "src/main/resources/native"
