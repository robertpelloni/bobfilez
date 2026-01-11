package com.filez.core.impl;

import com.filez.core.interfaces.FileScanner;
import com.filez.core.types.FileInfo;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.time.Instant;
import java.util.*;
import java.util.function.Consumer;

public class NioFileScanner implements FileScanner {

    @Override
    public List<FileInfo> scan(List<Path> roots, Set<String> extensions, boolean followSymlinks) {
        List<FileInfo> results = new ArrayList<>();
        Set<String> normalizedExtensions = normalizeExtensions(extensions);
        
        for (Path root : roots) {
            if (!Files.exists(root)) {
                continue;
            }
            
            Set<FileVisitOption> options = followSymlinks 
                ? EnumSet.of(FileVisitOption.FOLLOW_LINKS) 
                : EnumSet.noneOf(FileVisitOption.class);
            
            try {
                Files.walkFileTree(root, options, Integer.MAX_VALUE, new SimpleFileVisitor<>() {
                    @Override
                    public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) {
                        if (attrs.isRegularFile() && matchesExtension(file, normalizedExtensions)) {
                            results.add(toFileInfo(file, attrs));
                        }
                        return FileVisitResult.CONTINUE;
                    }
                    
                    @Override
                    public FileVisitResult visitFileFailed(Path file, IOException exc) {
                        return FileVisitResult.CONTINUE;
                    }
                });
            } catch (IOException e) {
                throw new UncheckedIOException("Failed to scan directory: " + root, e);
            }
        }
        
        return results;
    }

    @Override
    public void scanAsync(
            List<Path> roots,
            Set<String> extensions,
            boolean followSymlinks,
            Consumer<FileInfo> onFile,
            Consumer<Path> onDirectory) {
        
        Set<String> normalizedExtensions = normalizeExtensions(extensions);
        
        for (Path root : roots) {
            if (!Files.exists(root)) {
                continue;
            }
            
            Set<FileVisitOption> options = followSymlinks 
                ? EnumSet.of(FileVisitOption.FOLLOW_LINKS) 
                : EnumSet.noneOf(FileVisitOption.class);
            
            try {
                Files.walkFileTree(root, options, Integer.MAX_VALUE, new SimpleFileVisitor<>() {
                    @Override
                    public FileVisitResult preVisitDirectory(Path dir, BasicFileAttributes attrs) {
                        if (onDirectory != null) {
                            onDirectory.accept(dir);
                        }
                        return FileVisitResult.CONTINUE;
                    }
                    
                    @Override
                    public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) {
                        if (attrs.isRegularFile() && matchesExtension(file, normalizedExtensions)) {
                            if (onFile != null) {
                                onFile.accept(toFileInfo(file, attrs));
                            }
                        }
                        return FileVisitResult.CONTINUE;
                    }
                    
                    @Override
                    public FileVisitResult visitFileFailed(Path file, IOException exc) {
                        return FileVisitResult.CONTINUE;
                    }
                });
            } catch (IOException e) {
                throw new UncheckedIOException("Failed to scan directory: " + root, e);
            }
        }
    }

    private Set<String> normalizeExtensions(Set<String> extensions) {
        if (extensions == null || extensions.isEmpty()) {
            return Set.of();
        }
        Set<String> normalized = new HashSet<>();
        for (String ext : extensions) {
            String e = ext.toLowerCase();
            if (!e.startsWith(".")) {
                e = "." + e;
            }
            normalized.add(e);
        }
        return normalized;
    }

    private boolean matchesExtension(Path file, Set<String> extensions) {
        if (extensions.isEmpty()) {
            return true;
        }
        String name = file.getFileName().toString().toLowerCase();
        for (String ext : extensions) {
            if (name.endsWith(ext)) {
                return true;
            }
        }
        return false;
    }

    private FileInfo toFileInfo(Path file, BasicFileAttributes attrs) {
        return FileInfo.of(
            file,
            attrs.size(),
            attrs.lastModifiedTime().toInstant(),
            attrs.isDirectory()
        );
    }
}
