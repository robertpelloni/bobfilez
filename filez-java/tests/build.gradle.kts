plugins {
    java
}

dependencies {
    // Core and CLI modules under test
    implementation(project(":core"))
    implementation(project(":cli"))
    
    // JUnit 5 BOM for version alignment
    testImplementation(platform("org.junit:junit-bom:5.10.2"))
    
    // JUnit 5 Jupiter
    testImplementation("org.junit.jupiter:junit-jupiter")
    
    // Assertions
    testImplementation("org.assertj:assertj-core:3.25.3")
    
    // Mockito for mocking (updated to valid version)
    testImplementation("org.mockito:mockito-core:5.12.0")
}

// Configure test task to use JUnit 5 platform
tasks.test {
    useJUnitPlatform()
    
    // Show standard output and error streams
    testLogging {
        events("passed", "skipped", "failed")
        showStandardStreams = false
    }
    
    // Set JVM options for tests
    jvmArgs("-Xmx512m", "-Xms128m")
}

// Ensure Java 21 compatibility
java {
    sourceCompatibility = JavaVersion.VERSION_21
    targetCompatibility = JavaVersion.VERSION_21
}
