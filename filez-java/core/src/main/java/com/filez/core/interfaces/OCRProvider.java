package com.filez.core.interfaces;

import com.filez.core.types.OCRResult;
import java.nio.file.Path;
import java.util.Optional;

public interface OCRProvider {
    
    Optional<OCRResult> recognize(Path path, String language);
    
    default Optional<OCRResult> recognize(Path path) {
        return recognize(path, "eng");
    }
    
    boolean isAvailable();
    
    String[] supportedLanguages();
}
