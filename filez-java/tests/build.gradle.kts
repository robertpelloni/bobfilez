dependencies {
    implementation(project(':core'))
    implementation(project(':cli'))
    
    // JUnit 5 - mockito core
    testImplementation('org.mockito:mockito-core:5.5.3')
    
    testImplementation('org.junit.jupiter:junit-jupiter')
    testImplementation('org.assertj:assertj-core:3.25.3')
}
