plugins {
    java
    application
}

dependencies {
    // Core and CLI modules
    implementation(project(":core"))
    implementation(project(":cli"))
    
    // JavaFX dependencies
    implementation("org.openjfx:javafx-controls:21.0.2")
    implementation("org.openjfx:javafx-fxml:21.0.2")
    implementation("org.openjfx:javafx-swing:21.0.2")
    
    // Testing
    testImplementation(platform("org.junit:junit-bom:5.10.2"))
    testImplementation("org.junit.jupiter:junit-jupiter")
    testImplementation("org.assertj:assertj-core:3.25.3")
}

application {
    mainClass.set("com.filez.gui.FilezApplication")
    
    applicationDefaultJvmArgs = listOf(
        "-Dprism.forceGPU=false",
        "-Dprism.verbose=true",
        "-Xmx256m",
        "-Xms128m"
    )
}

// Java compilation settings
tasks.withType<JavaCompile> {
    options.encoding = "UTF-8"
}

// Create distribution JAR with all dependencies
tasks.register<Jar>("distJar") {
    description = "Creates distribution JAR with dependencies"
    archiveClassifier.set("all")
    duplicatesStrategy = DuplicatesStrategy.EXCLUDE
    
    manifest {
        attributes["Main-Class"] = "com.filez.gui.FilezApplication"
    }
    
    from(sourceSets.main.get().output)
    
    dependsOn(configurations.runtimeClasspath)
    from({
        configurations.runtimeClasspath.get()
            .filter { it.name.endsWith("jar") }
            .map { zipTree(it) }
    })
}
