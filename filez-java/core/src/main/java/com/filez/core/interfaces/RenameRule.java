package com.filez.core.interfaces;

import java.nio.file.Path;
import java.util.List;
import java.util.Map;

/**
 * Rule-based batch renaming interface.
 */
public interface RenameRule {
    String getType();
    String apply(String stem, int index, Path originalPath, Map<String, String> metadata);
    
    record RenamePreview(
        Path original,
        Path proposed,
        boolean conflict,
        String error
    ) {}

    record RenameResult(
        Path original,
        Path destination,
        boolean success,
        String error
    ) {}
}
