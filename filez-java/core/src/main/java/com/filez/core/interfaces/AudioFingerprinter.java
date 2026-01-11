package com.filez.core.interfaces;

import com.filez.core.types.AudioFingerprint;
import java.nio.file.Path;
import java.util.Optional;

public interface AudioFingerprinter {
    
    Optional<AudioFingerprint> compute(Path path);
    
    String algorithm();
    
    double compare(AudioFingerprint a, AudioFingerprint b);
    
    default double defaultThreshold() {
        return 0.85;
    }
}
