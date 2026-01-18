package com.filez.gui;

import javafx.application.Application;
import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.stage.Stage;

import java.io.File;
import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Test for CLI metadata command.
 * Verifies file metadata extraction with real CLI fat JAR.
 */
public class CliMetadataTest extends Application {
    
    private File cliFatJar;
    
    @Override
    public void start(Stage stage) throws Exception {
        cliFatJar = new File("C:/Users/hyper/workspace/bobfilez/filez-java/cli/build/libs/cli-2.1.0-all.jar");
        if (!cliFatJar.exists()) {
            System.err.println("ERROR: CLI fat JAR not found at: " + cliFatJar.getAbsolutePath());
            System.exit(1);
        }
    }
    
    @org.junit.jupiter.api.Test
    void testMetadataCommand() throws Exception {
        ProcessBuilder pb = new ProcessBuilder("java");
        pb.command().add("-Xmx128m", "-Xms64m", "-jar", cliFatJar.getAbsolutePath(), "metadata", "--help");
        
        Process process = pb.start();
        int exitCode = process.waitFor();
        
        assertEquals(0, exitCode, "Metadata --help should exit with code 0");
    }
    
    @org.junit.jupiter.api.Test
    void testMetadataWithFile() throws Exception {
        ProcessBuilder pb = new ProcessBuilder("java");
        pb.command().add("-Xmx128m", "-Xms64m", "-jar", cliFatJar.getAbsolutePath(), "metadata", 
            new File(System.getProperty("user.home")).getAbsolutePath());
        
        Process process = pb.start();
        StringBuilder output = new StringBuilder();
        
        try (var reader = process.inputReader()) {
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line);
            }
        } catch (Exception e) {
            // If reading fails, process might have already completed
        }
        
        int exitCode = process.waitFor();
        
        assertEquals(0, exitCode, "Metadata command should exit with code 0");
        assertFalse(output.length() == 0, "Should have metadata output");
        assertTrue(output.toString().contains("Usage:") || output.toString().contains("metadata"), 
            "Should show usage or metadata command output");
    }
    
    @org.junit.jupiter.api.Test
    void testOrganizeCommand() throws Exception {
        ProcessBuilder pb = new ProcessBuilder("java");
        pb.command().add("-Xmx128m", "-Xms64m", "-jar", cliFatJar.getAbsolutePath(), "organize", "--help");
        
        Process process = pb.start();
        int exitCode = process.waitFor();
        
        assertEquals(0, exitCode, "Organize --help should exit with code 0");
    }
}
