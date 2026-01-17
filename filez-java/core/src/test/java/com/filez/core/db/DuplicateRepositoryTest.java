package com.filez.core.db;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Nested;

import java.nio.file.Files;
import java.nio.file.Path;
import java.sql.SQLException;
import java.util.List;
import java.util.Optional;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.Mockito.*;

/**
 * Tests for DuplicateRepository.
 */
@DisplayName("DuplicateRepository Tests")
public class DuplicateRepositoryTest {
    
    private DatabaseManager dbManager;
    private DuplicateRepository duplicateRepository;
    private FileRepository fileRepository;
    private Path tempDb;
    
    @BeforeEach
    void setUp() throws Exception {
        tempDb = Files.createTempFile("filez-test", ".db");
        dbManager = DatabaseManager.inMemory(tempDb.toString());
        duplicateRepository = new DuplicateRepository(dbManager);
        fileRepository = new FileRepository(dbManager);
    }
    
    @Test
    @DisplayName("Should save and retrieve duplicate group")
    void shouldSaveAndRetrieveDuplicateGroup() throws SQLException {
        // First, save some files
        var fileInfo1 = new com.filez.core.types.FileInfo(
            Path.of("/path/to/file1.jpg"),
            1024L,
            "image/jpeg",
            Optional.empty()
        );
        var fileInfo2 = new com.filez.core.types.FileInfo(
            Path.of("/path/to/file2.jpg"),
            1024L,
            "image/jpeg",
            Optional.empty()
        );
        var fileInfo3 = new com.filez.core.types.FileInfo(
            Path.of("/path/to/file3.jpg"),
            1024L,
            "image/jpeg",
            Optional.empty()
        );
        
        fileRepository.save(fileInfo1);
        fileRepository.save(fileInfo2);
        fileRepository.save(fileInfo3);
        
        // Create and save a duplicate group
        var hashes = new com.filez.core.types.Hashes(
            "abc123",
            Optional.of("def456"),
            Optional.empty()
        );
        var duplicateGroup = new com.filez.core.types.DuplicateGroup(
            hashes,
            List.of(fileInfo1.path(), fileInfo2.path(), fileInfo3.path()),
            1024L,
            Optional.empty()
        );
        
        var savedId = duplicateRepository.save(duplicateGroup);
        assertThat(savedId).isPositive();
        
        // Retrieve the saved group
        var retrieved = duplicateRepository.findByHash(hashes.blake3());
        assertThat(retrieved).hasSize(1);
        assertThat(retrieved.get(0).fileSize()).isEqualTo(1024L);
        assertThat(retrieved.get(0).filePaths())
            .containsExactlyInAnyOrder(
                fileInfo1.path(), fileInfo2.path(), fileInfo3.path()
            );
    }
    
    @Test
    @DisplayName("Should find duplicate group by hash")
    void shouldFindDuplicateGroupByHash() throws SQLException {
        var fileInfo = new com.filez.core.types.FileInfo(
            Path.of("/path/to/file.jpg"),
            2048L,
            "image/jpeg",
            Optional.empty()
        );
        fileRepository.save(fileInfo);
        
        var hashes = new com.filez.core.types.Hashes(
            "searchHash123",
            Optional.empty(),
            Optional.empty()
        );
        var duplicateGroup = new com.filez.core.types.DuplicateGroup(
            hashes,
            List.of(fileInfo.path()),
            2048L,
            Optional.empty()
        );
        duplicateRepository.save(duplicateGroup);
        
        var result = duplicateRepository.findByHash("searchHash123");
        assertThat(result).isPresent();
        assertThat(result.get().fileSize()).isEqualTo(2048L);
        assertThat(result.get().filePaths()).containsExactly(fileInfo.path());
    }
    
    @Test
    @DisplayName("Should return empty when hash not found")
    void shouldReturnEmptyWhenHashNotFound() throws SQLException {
        var result = duplicateRepository.findByHash("nonexistentHash");
        assertThat(result).isEmpty();
    }
    
    @Test
    @DisplayName("Should get all duplicate groups")
    void shouldGetAllDuplicateGroups() throws SQLException {
        var fileInfo1 = new com.filez.core.types.FileInfo(
            Path.of("/file1.jpg"),
            1024L,
            "image/jpeg",
            Optional.empty()
        );
        var fileInfo2 = new com.filez.core.types.FileInfo(
            Path.of("/file2.png"),
            2048L,
            "image/png",
            Optional.empty()
        );
        fileRepository.save(fileInfo1);
        fileRepository.save(fileInfo2);
        
        var hashes1 = new com.filez.core.types.Hashes(
            "hash111",
            Optional.empty(),
            Optional.empty()
        );
        var hashes2 = new com.filez.core.types.Hashes(
            "hash222",
            Optional.empty(),
            Optional.empty()
        );
        var duplicateGroup1 = new com.filez.core.types.DuplicateGroup(
            hashes1,
            List.of(fileInfo1.path()),
            1024L,
            Optional.empty()
        );
        var duplicateGroup2 = new com.filez.core.types.DuplicateGroup(
            hashes2,
            List.of(fileInfo2.path()),
            2048L,
            Optional.empty()
        );
        duplicateRepository.save(duplicateGroup1);
        duplicateRepository.save(duplicateGroup2);
        
        var allGroups = duplicateRepository.findAll();
        assertThat(allGroups).hasSize(2);
    }
    
    @Test
    @DisplayName("Should delete duplicate group by ID")
    void shouldDeleteDuplicateGroupById() throws SQLException {
        var fileInfo = new com.filez.core.types.FileInfo(
            Path.of("/file.jpg"),
            1024L,
            "image/jpeg",
            Optional.empty()
        );
        fileRepository.save(fileInfo);
        
        var hashes = new com.filez.core.types.Hashes(
            "deleteHash123",
            Optional.empty(),
            Optional.empty()
        );
        var duplicateGroup = new com.filez.core.types.DuplicateGroup(
            hashes,
            List.of(fileInfo.path()),
            1024L,
            Optional.empty()
        );
        var savedId = duplicateRepository.save(duplicateGroup);
        
        // Delete the group
        duplicateRepository.deleteById(savedId);
        
        // Verify it's gone
        var result = duplicateRepository.findByHash("deleteHash123");
        assertThat(result).isEmpty();
    }
    
    @Test
    @DisplayName("Should handle null values gracefully")
    void shouldHandleNullValues() throws SQLException {
        var fileInfo = new com.filez.core.types.FileInfo(
            Path.of("/file.jpg"),
            1024L,
            "image/jpeg",
            Optional.empty()
        );
        fileRepository.save(fileInfo);
        
        // Hashes with null values
        var hashes = new com.filez.core.types.Hashes(
            "nullHash",
            null,
            null
        );
        var duplicateGroup = new com.filez.core.types.DuplicateGroup(
            hashes,
            List.of(fileInfo.path()),
            1024L,
            null
        );
        
        var savedId = duplicateRepository.save(duplicateGroup);
        assertThat(savedId).isPositive();
        
        var retrieved = duplicateRepository.findById(savedId);
        assertThat(retrieved).isPresent();
        assertThat(retrieved.get().sha256()).isEmpty();
        assertThat(retrieved.get().md5()).isEmpty();
    }
}
