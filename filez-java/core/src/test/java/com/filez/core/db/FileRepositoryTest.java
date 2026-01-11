package com.filez.core.db;

import com.filez.core.types.FileInfo;
import com.filez.core.types.Hashes;
import org.junit.jupiter.api.*;

import java.nio.file.Path;
import java.sql.SQLException;
import java.time.Instant;
import java.util.List;
import java.util.Optional;

import static org.junit.jupiter.api.Assertions.*;

class FileRepositoryTest {

    private DatabaseManager db;
    private FileRepository repo;

    @BeforeEach
    void setUp() throws SQLException {
        db = DatabaseManager.inMemory();
        repo = new FileRepository(db);
    }

    @AfterEach
    void tearDown() throws SQLException {
        db.close();
    }

    private FileInfo createFile(String path, long size) {
        return new FileInfo(0, Path.of(path), size, Instant.now(), false);
    }

    @Test
    void saveAndFindById() throws SQLException {
        FileInfo file = createFile("/test/file.txt", 100);
        FileInfo saved = repo.save(file);

        assertTrue(saved.id() > 0);

        Optional<FileInfo> found = repo.findById(saved.id());
        assertTrue(found.isPresent());
        assertEquals(saved.id(), found.get().id());
        assertEquals(file.path(), found.get().path());
        assertEquals(file.size(), found.get().size());
    }

    @Test
    void saveAndFindByPath() throws SQLException {
        FileInfo file = createFile("/test/file.txt", 100);
        repo.save(file);

        Optional<FileInfo> found = repo.findByPath(Path.of("/test/file.txt"));
        assertTrue(found.isPresent());
        assertEquals(100, found.get().size());
    }

    @Test
    void findByPathNotFound() throws SQLException {
        Optional<FileInfo> found = repo.findByPath(Path.of("/nonexistent"));
        assertTrue(found.isEmpty());
    }

    @Test
    void saveUpdatesExisting() throws SQLException {
        FileInfo file1 = createFile("/test/file.txt", 100);
        FileInfo saved1 = repo.save(file1);

        FileInfo file2 = createFile("/test/file.txt", 200);
        FileInfo saved2 = repo.save(file2);

        assertEquals(saved1.id(), saved2.id());

        Optional<FileInfo> found = repo.findById(saved1.id());
        assertTrue(found.isPresent());
        assertEquals(200, found.get().size());
    }

    @Test
    void findBySize() throws SQLException {
        repo.save(createFile("/a.txt", 100));
        repo.save(createFile("/b.txt", 100));
        repo.save(createFile("/c.txt", 200));

        List<FileInfo> results = repo.findBySize(100);
        assertEquals(2, results.size());
    }

    @Test
    void findAll() throws SQLException {
        repo.save(createFile("/a.txt", 10));
        repo.save(createFile("/b.txt", 20));
        repo.save(createFile("/c.txt", 30));

        List<FileInfo> results = repo.findAll();
        assertEquals(3, results.size());
    }

    @Test
    void deleteById() throws SQLException {
        FileInfo saved = repo.save(createFile("/test.txt", 100));
        repo.delete(saved.id());

        Optional<FileInfo> found = repo.findById(saved.id());
        assertTrue(found.isEmpty());
    }

    @Test
    void deleteByPath() throws SQLException {
        repo.save(createFile("/test.txt", 100));
        repo.deleteByPath(Path.of("/test.txt"));

        Optional<FileInfo> found = repo.findByPath(Path.of("/test.txt"));
        assertTrue(found.isEmpty());
    }

    @Test
    void count() throws SQLException {
        assertEquals(0, repo.count());

        repo.save(createFile("/a.txt", 10));
        repo.save(createFile("/b.txt", 20));

        assertEquals(2, repo.count());
    }

    @Test
    void saveBatch() throws SQLException {
        List<FileInfo> files = List.of(
            createFile("/a.txt", 10),
            createFile("/b.txt", 20),
            createFile("/c.txt", 30)
        );

        repo.saveBatch(files);

        assertEquals(3, repo.count());
    }

    @Test
    void saveAndFindHashes() throws SQLException {
        FileInfo saved = repo.save(createFile("/test.txt", 100));
        Hashes hashes = Hashes.of("abc123", "sha256hash");

        repo.saveHashes(saved.id(), hashes);

        Optional<Hashes> found = repo.findHashes(saved.id());
        assertTrue(found.isPresent());
        assertEquals("abc123", found.get().fast64());
        assertEquals("sha256hash", found.get().strong().orElse(null));
    }

    @Test
    void findHashesNotFound() throws SQLException {
        Optional<Hashes> found = repo.findHashes(999);
        assertTrue(found.isEmpty());
    }

    @Test
    void saveHashesWithoutStrong() throws SQLException {
        FileInfo saved = repo.save(createFile("/test.txt", 100));
        Hashes hashes = Hashes.ofFast("abc123");

        repo.saveHashes(saved.id(), hashes);

        Optional<Hashes> found = repo.findHashes(saved.id());
        assertTrue(found.isPresent());
        assertEquals("abc123", found.get().fast64());
        assertTrue(found.get().strong().isEmpty());
    }

    @Test
    void directoryFlag() throws SQLException {
        FileInfo dir = new FileInfo(0, Path.of("/mydir"), 0, Instant.now(), true);
        FileInfo saved = repo.save(dir);

        Optional<FileInfo> found = repo.findById(saved.id());
        assertTrue(found.isPresent());
        assertTrue(found.get().isDirectory());
    }
}
