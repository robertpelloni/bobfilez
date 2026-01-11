package com.filez.core.interfaces;

import com.filez.core.types.FileInfo;
import java.nio.file.Path;
import java.util.List;
import java.util.Set;
import java.util.function.Consumer;

public interface FileScanner {
    
    List<FileInfo> scan(List<Path> roots, Set<String> extensions, boolean followSymlinks);
    
    void scanAsync(
        List<Path> roots, 
        Set<String> extensions, 
        boolean followSymlinks,
        Consumer<FileInfo> onFile,
        Consumer<Path> onDirectory
    );
    
    default List<FileInfo> scan(Path root, Set<String> extensions) {
        return scan(List.of(root), extensions, false);
    }
    
    default List<FileInfo> scanAll(Path root) {
        return scan(List.of(root), Set.of(), false);
    }
}
