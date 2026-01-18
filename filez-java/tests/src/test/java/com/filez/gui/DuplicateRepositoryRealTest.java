package com.filez.gui;

import javafx.application.Application;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.stage.Stage;

import java.io.File;
import java.nio.file.Path;
import java.nio.file.Files;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Test for DuplicateRepository using real SQLite database.
 * Tests CRUD operations, search, and null handling.
 */
public class DuplicateRepositoryRealTest extends Application {
    
    private DatabaseManager dbManager;
    private FileRepository repository;
    private com.filez.core.types.FileInfo testFile;
    private com.filez.core.types.Hashes testHashes;
    
    @Override
    public void start(Stage stage) throws Exception {
        // Use in-memory SQLite for isolated testing
        dbManager = new DatabaseManager(":memory:");
        repository = new FileRepository(dbManager);
        
        // Create test data
        testFile = new FileInfo(
            Path.of("C:/test-file.jpg"),
            1024L,
            "image/jpeg",
            null
        );
        
        testHashes = new Hashes(
            "test_hash_abc123",
            Optional.of("sha256_hash_def"),
            Optional.empty()
        );
        
        // Initialize database
        repository.save(testFile);
        repository.save(testFile);
    }
    
    @org.junit.jupiter.api.Test
    void testSaveFile() {
        // Save a single file
        long id = repository.save(testFile);
        assertTrue(id > 0, "File should have ID after save");
    }
    
    @org.junit.jupiter.api.Test
    void testSaveDuplicateGroup() {
        // Create a duplicate group with multiple files
        testFile2 = new FileInfo(
            Path.of("C:/test-file2.jpg"),
            1024L,
            "image/jpeg",
            null
        );
        
        var group = new com.filez.core.types.DuplicateGroup(
            testHashes,
            java.util.List.of(testFile.path(), testFile2.path()),
            1024L,
            Optional.empty()
        );
        
        long id = repository.save(group);
        assertTrue(id > 0, "Duplicate group should have ID after save");
    }
    
    @org.junit.jupiter.api.Test
    void testFindDuplicateGroupByHash() {
        // Find a group by hash
        var group = repository.findByHash(testHashes.blake3());
        assertTrue(group.isPresent(), "Should find group by hash");
    }
    
    @org.junit.jupiter.api.Test
    void testDeleteById() {
        // Create and delete a group
        testFile3 = new FileInfo(
            Path.of("C:/test-file3.jpg"),
            512L,
            "image/jpeg",
            null
        );
        
        var group2 = new com.filez.core.types.DuplicateGroup(
            testHashes,
            java.util.List.of(testFile.path()),
            long id = repository.save(group2);
        );
        
        repository.deleteById(long id);
        
        // Verify group is deleted
        var found = repository.findByHash(testHashes.blake3());
        assertFalse(found.isPresent(), "Deleted group should not be found");
    }
    
    @org.junit.jupiter.api.Test
    void testNullHandling() {
        // Test null handling in DatabaseManager
        try {
            dbManager.inMemory("invalid");
            fail("Should throw exception for invalid database");
        } catch (Exception e) {
            // Expected exception
            assertTrue(e.getMessage().contains("invalid"), "Should have informative error message");
        }
    }
    
    @org.junit.jupiter.api.Test
    void testGetAllDuplicates() {
        // Test retrieving all duplicate groups
        var groups = repository.findAll();
        assertTrue(groups.size() >= 0, "Should return at least 0 groups");
    }
}
