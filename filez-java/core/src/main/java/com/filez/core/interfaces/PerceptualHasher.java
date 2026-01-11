package com.filez.core.interfaces;

import com.filez.core.types.PerceptualHash;
import java.nio.file.Path;
import java.util.Optional;

public interface PerceptualHasher {
    
    Optional<PerceptualHash> compute(Path path);
    
    String method();
    
    default int defaultThreshold() {
        return 10;
    }
}
