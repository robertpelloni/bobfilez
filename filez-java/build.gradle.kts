plugins {
    java
    idea
}

allprojects {
    group = "com.filez"
    version = "2.1.0"
    
    repositories {
        mavenCentral()
    }
    
    subprojects {
        project(":gui") {
            apply(plugin = "javafx")
        }
        
        project(":cli") {
            apply(plugin = "application")
        }
        
        project(":core") {
            apply(plugin = "java-library")
        }
        
        project(":native") {
            apply(plugin = "java-library")
        }
    }
}

dependencies {
    implementation(project(":core"))
    implementation(project(":cli"))
    
    implementation("org.openjfx:javafx-controls:21.0.2")
    implementation("org.openjfx:javafx-fxml:21.0.2")
    
    implementation("org.openjfx:javafx-swing:21.0.2")
    
    testImplementation(platform("org.junit:junit-bom:5.10.2"))
    testImplementation("org.junit.jupiter:junit-jupiter")
    testImplementation("org.assertj:assertj-core:3.25.3")
    testImplementation("org.testfx:junit-jupiter:5.11.1")
}

application {
    mainClass.set("com.filez.gui.FilezApplication")
    
    applicationDefaultJvmArgs = listOf(
        "-Dprism.forceGPU=false",
        "-Dprism.verbose=true"
    )
    
    application {
        name = "filez"
        
        jvmArgs = listOf(
            "-Xmx256m",
            "-Xms64m"
        )
    }
}
