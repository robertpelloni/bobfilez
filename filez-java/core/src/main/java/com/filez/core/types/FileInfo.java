package com.filez.core.types;

import java.nio.file.Path;
import java.time.Instant;
import java.util.Objects;

/**
 * Core file information record.
 * Immutable representation of a file's metadata.
 */
public record FileInfo(
    long id,
    Path path,
    long size,
    Instant modifiedTime,
    boolean isDirectory
) {
    public FileInfo {
        Objects.requireNonNull(path, "path must not be null");
    }

    /**
     * Creates a FileInfo without an ID (for newly discovered files).
     */
    public static FileInfo of(Path path, long size, Instant modifiedTime, boolean isDirectory) {
        return new FileInfo(-1, path, size, modifiedTime, isDirectory);
    }

    /**
     * Creates a copy with the specified ID.
     */
    public FileInfo withId(long id) {
        return new FileInfo(id, path, size, modifiedTime, isDirectory);
    }

    /**
     * Returns the file name without the directory path.
     */
    public String fileName() {
        return path.getFileName().toString();
    }

    /**
     * Returns the file extension (lowercase, without dot).
     */
    public String extension() {
        String name = fileName();
        int dot = name.lastIndexOf('.');
        return dot > 0 ? name.substring(dot + 1).toLowerCase() : "";
    }
}
