plugins {
    id 'filez-gui'
}

apply plugin: 'javafx'

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

apply plugin: 'application'

javafx {
    version = '21.0.2'
    modules = ['com.filez.core', 'com.filez.cli']
    mainClass = 'com.filez.gui.FilezApplication'
    
    applicationDefaultJvmArgs = [
        '-Dprism.forceGPU=false',
        '-Dprism.verbose=true'
    ]
    
    application {
        name = 'filez'
        
        jvmArgs = [
            '-Xmx256m',
            '-Xms128m'
        ]
    }
}

tasks.withType(JavaCompile) {
    options.encoding = 'UTF-8'
}

tasks.withType(JavaExec) {
    mainClass = 'com.filez.gui.FilezApplication'
}

task runCli(type: JavaExec) {
    description = 'Runs CLI commands (for testing CLI integration)'
    classpath = sourceSets.main.output + configurations.runtimeClasspath
    mainClass = 'com.filez.cli.FilezApp'
}

task distJar(type: Jar) {
    description = 'Creates distribution JAR with dependencies'
    archiveClassifier = 'all'
    duplicatesStrategy = 'exclude'
    
    manifest {
        attributes('Main-Class': 'com.filez.gui.FilezApplication')
    }
    
    from sourceSets.main.output
    
    dependsOn(configurations.runtimeClasspath)
    from configurations.runtimeClasspath.filter { it.name.endsWith('jar') }.collect { zipTree(it) }
}
