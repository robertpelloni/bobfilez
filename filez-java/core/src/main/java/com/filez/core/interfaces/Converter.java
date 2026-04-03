package com.filez.core.interfaces;

import java.nio.file.Path;
import java.util.Map;
import java.util.Optional;

/**
 * Universal file format conversion interface.
 */
public interface Converter {
    String getName();
    String getDescription();
    
    boolean canHandle(String fromExt, String toExt);
    
    record ConversionResult(
        Path input,
        Path output,
        boolean success,
        String error,
        long durationMs
    ) {}

    ConversionResult convert(Path input, Path output, Map<String, String> options);
}
