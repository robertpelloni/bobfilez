package com.filez.core.types;

import java.nio.file.Path;
import java.util.List;
import java.util.Objects;

/**
 * A group of duplicate files sharing the same content.
 */
public record DuplicateGroup(
    long size,           // File size (all duplicates have same size)
    String fast64,       // Fast hash (all duplicates have same hash)
    List<Path> files     // List of duplicate file paths
) {
    public DuplicateGroup {
        Objects.requireNonNull(fast64, "fast64 must not be null");
        Objects.requireNonNull(files, "files must not be null");
        if (files.size() < 2) {
            throw new IllegalArgumentException("A duplicate group must have at least 2 files");
        }
    }

    /**
     * Returns the number of duplicates (excluding the original).
     */
    public int duplicateCount() {
        return files.size() - 1;
    }

    /**
     * Returns the total wasted space (size * duplicateCount).
     */
    public long wastedSpace() {
        return size * duplicateCount();
    }

    /**
     * Returns the first file (typically considered the "original").
     */
    public Path original() {
        return files.getFirst();
    }

    /**
     * Returns all files except the first (the duplicates).
     */
    public List<Path> duplicates() {
        return files.subList(1, files.size());
    }
}
