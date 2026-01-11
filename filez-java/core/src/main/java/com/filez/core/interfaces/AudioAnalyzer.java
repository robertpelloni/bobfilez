package com.filez.core.interfaces;

import com.filez.core.types.AudioQualityAnalysis;
import java.nio.file.Path;
import java.util.Optional;

public interface AudioAnalyzer {
    
    Optional<AudioQualityAnalysis> analyze(Path path);
    
    boolean canHandle(Path path);
    
    double calculateRating(AudioQualityAnalysis analysis);
}
