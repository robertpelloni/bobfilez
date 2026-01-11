package com.filez.core.impl;

import com.filez.core.types.Hashes;
import org.junit.jupiter.api.*;
import org.junit.jupiter.api.io.TempDir;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Optional;

import static org.junit.jupiter.api.Assertions.*;

class JavaHasherTest {

    @TempDir
    Path tempDir;

    private JavaHasher hasher;

    @BeforeEach
    void setUp() {
        hasher = new JavaHasher();
    }

    @Test
    void fast64ReturnsHexString() throws IOException {
        Path file = tempDir.resolve("test.txt");
        Files.writeString(file, "hello world");

        String hash = hasher.fast64(file);

        assertNotNull(hash);
        assertTrue(hash.length() <= 8);
        assertTrue(hash.matches("[0-9a-f]+"));
    }

    @Test
    void fast64IsDeterministic() throws IOException {
        Path file = tempDir.resolve("test.txt");
        Files.writeString(file, "hello world");

        String hash1 = hasher.fast64(file);
        String hash2 = hasher.fast64(file);

        assertEquals(hash1, hash2);
    }

    @Test
    void fast64DifferentContentDifferentHash() throws IOException {
        Path file1 = tempDir.resolve("file1.txt");
        Path file2 = tempDir.resolve("file2.txt");
        Files.writeString(file1, "content A");
        Files.writeString(file2, "content B");

        String hash1 = hasher.fast64(file1);
        String hash2 = hasher.fast64(file2);

        assertNotEquals(hash1, hash2);
    }

    @Test
    void strongReturnsSha256() throws IOException {
        Path file = tempDir.resolve("test.txt");
        Files.writeString(file, "hello world");

        Optional<String> hash = hasher.strong(file);

        assertTrue(hash.isPresent());
        assertEquals(64, hash.get().length());
        assertTrue(hash.get().matches("[0-9a-f]+"));
    }

    @Test
    void strongKnownValue() throws IOException {
        Path file = tempDir.resolve("known.txt");
        Files.writeString(file, "hello");

        Optional<String> hash = hasher.strong(file);

        assertTrue(hash.isPresent());
        assertEquals("2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824", hash.get());
    }

    @Test
    void hashReturnsHashesObject() throws IOException {
        Path file = tempDir.resolve("test.txt");
        Files.writeString(file, "test content");

        Hashes hashes = hasher.hash(file);

        assertNotNull(hashes);
        assertNotNull(hashes.fast64());
        assertTrue(hashes.strong().isPresent());
    }

    @Test
    void emptyFileHashes() throws IOException {
        Path file = tempDir.resolve("empty.txt");
        Files.writeString(file, "");

        Hashes hashes = hasher.hash(file);

        assertNotNull(hashes.fast64());
        assertTrue(hashes.strong().isPresent());
    }

    @Test
    void largeFileHashes() throws IOException {
        Path file = tempDir.resolve("large.bin");
        byte[] data = new byte[1024 * 1024];
        for (int i = 0; i < data.length; i++) {
            data[i] = (byte) (i % 256);
        }
        Files.write(file, data);

        Hashes hashes = hasher.hash(file);

        assertNotNull(hashes.fast64());
        assertTrue(hashes.strong().isPresent());
    }
}
