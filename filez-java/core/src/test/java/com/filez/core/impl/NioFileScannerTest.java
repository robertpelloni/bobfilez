package com.filez.core.impl;

import com.filez.core.types.FileInfo;
import org.junit.jupiter.api.*;
import org.junit.jupiter.api.io.TempDir;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

class NioFileScannerTest {

    @TempDir
    Path tempDir;

    private NioFileScanner scanner;

    @BeforeEach
    void setUp() {
        scanner = new NioFileScanner();
    }

    private List<FileInfo> scan(Path root) {
        return scanner.scan(List.of(root), Set.of(), false);
    }

    private List<FileInfo> scan(Path root, Set<String> extensions) {
        return scanner.scan(List.of(root), extensions, false);
    }

    @Test
    void scanEmptyDirectory() {
        List<FileInfo> results = scan(tempDir);
        assertTrue(results.isEmpty());
    }

    @Test
    void scanSingleFile() throws IOException {
        Path file = tempDir.resolve("test.txt");
        Files.writeString(file, "hello world");

        List<FileInfo> results = scan(tempDir);

        assertEquals(1, results.size());
        FileInfo info = results.get(0);
        assertEquals(file, info.path());
        assertEquals(11, info.size());
        assertFalse(info.isDirectory());
    }

    @Test
    void scanMultipleFiles() throws IOException {
        Files.writeString(tempDir.resolve("a.txt"), "aaa");
        Files.writeString(tempDir.resolve("b.txt"), "bbbbb");
        Files.writeString(tempDir.resolve("c.txt"), "ccccccc");

        List<FileInfo> results = scan(tempDir);

        assertEquals(3, results.size());
        Set<Long> sizes = results.stream().map(FileInfo::size).collect(Collectors.toSet());
        assertEquals(Set.of(3L, 5L, 7L), sizes);
    }

    @Test
    void scanNestedDirectories() throws IOException {
        Path subDir = tempDir.resolve("sub");
        Files.createDirectory(subDir);
        Files.writeString(tempDir.resolve("root.txt"), "root");
        Files.writeString(subDir.resolve("nested.txt"), "nested");

        List<FileInfo> results = scan(tempDir);

        assertEquals(2, results.size());
        Set<String> names = results.stream()
            .map(f -> f.path().getFileName().toString())
            .collect(Collectors.toSet());
        assertEquals(Set.of("root.txt", "nested.txt"), names);
    }

    @Test
    void scanWithExtensionFilter() throws IOException {
        Files.writeString(tempDir.resolve("doc.txt"), "text");
        Files.writeString(tempDir.resolve("image.jpg"), "image");
        Files.writeString(tempDir.resolve("data.txt"), "data");

        List<FileInfo> results = scan(tempDir, Set.of(".txt"));

        assertEquals(2, results.size());
        assertTrue(results.stream().allMatch(f -> f.path().toString().endsWith(".txt")));
    }

    @Test
    void scanNonExistentDirectoryReturnsEmpty() {
        Path nonExistent = tempDir.resolve("does-not-exist");
        List<FileInfo> results = scan(nonExistent);
        assertTrue(results.isEmpty());
    }

    @Test
    void fileInfoHasCorrectModifiedTime() throws IOException {
        Path file = tempDir.resolve("timed.txt");
        Files.writeString(file, "test");

        List<FileInfo> results = scan(tempDir);

        assertEquals(1, results.size());
        assertNotNull(results.get(0).modifiedTime());
        assertTrue(results.get(0).modifiedTime().toEpochMilli() > 0);
    }

    @Test
    void scanMultipleRoots() throws IOException {
        Path dir1 = tempDir.resolve("dir1");
        Path dir2 = tempDir.resolve("dir2");
        Files.createDirectories(dir1);
        Files.createDirectories(dir2);
        Files.writeString(dir1.resolve("a.txt"), "a");
        Files.writeString(dir2.resolve("b.txt"), "b");

        List<FileInfo> results = scanner.scan(List.of(dir1, dir2), Set.of(), false);

        assertEquals(2, results.size());
    }
}
