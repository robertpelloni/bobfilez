plugins {
    `java-library`
}

dependencies {
    // SQLite JDBC driver
    api("org.xerial:sqlite-jdbc:3.45.1.0")
    
    // Metadata extraction (EXIF, etc.)
    api("com.drewnoakes:metadata-extractor:2.19.0")
    
    // Null safety annotations
    compileOnly("org.jetbrains:annotations:24.1.0")
    
    // Testing
    testImplementation(platform("org.junit:junit-bom:5.10.2"))
    testImplementation("org.junit.jupiter:junit-jupiter")
    testImplementation("org.assertj:assertj-core:3.25.3")
}
