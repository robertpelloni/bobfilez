package com.filez.core.interfaces;

import com.filez.core.types.ClassificationResult;
import java.nio.file.Path;
import java.util.List;
import java.util.Optional;

public interface ImageClassifier {
    
    List<ClassificationResult> classify(Path path, int topK);
    
    default Optional<ClassificationResult> classifyTop(Path path) {
        List<ClassificationResult> results = classify(path, 1);
        return results.isEmpty() ? Optional.empty() : Optional.of(results.getFirst());
    }
    
    boolean isAvailable();
    
    String modelName();
}
