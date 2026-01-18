// Root build configuration for filez-java multi-module project
plugins {
    // No plugins at root level - subprojects define their own
}

allprojects {
    group = "com.filez"
    version = "2.1.0"
    
    repositories {
        mavenCentral()
    }
}

// Configure all subprojects with common Java settings
subprojects {
    // Apply java plugin to all subprojects
    apply(plugin = "java")
    
    // Common Java compilation settings
    configure<JavaPluginExtension> {
        sourceCompatibility = JavaVersion.VERSION_21
        targetCompatibility = JavaVersion.VERSION_21
    }
    
    // Common test configuration
    tasks.withType<Test> {
        useJUnitPlatform()
        testLogging {
            events("passed", "skipped", "failed")
        }
    }
}
