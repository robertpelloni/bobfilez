package com.filez.core.types;

import java.nio.file.Path;
import java.util.List;

public record AudioDuplicateGroup(
    List<AudioDuplicateFile> files,
    double tagScore,
    double contentScore,
    double preciseScore
) {
    public AudioDuplicateGroup {
        if (files == null || files.size() < 2) {
            throw new IllegalArgumentException("Duplicate group must have at least 2 files");
        }
    }

    public double bestScore() {
        return Math.max(Math.max(tagScore, contentScore), preciseScore);
    }

    public AudioDuplicateFile bestQuality() {
        return files.stream()
            .max((a, b) -> Double.compare(
                a.analysis() != null ? a.analysis().rating() : 0,
                b.analysis() != null ? b.analysis().rating() : 0
            ))
            .orElse(files.getFirst());
    }

    public List<AudioDuplicateFile> worstQuality() {
        AudioDuplicateFile best = bestQuality();
        return files.stream()
            .filter(f -> f != best)
            .toList();
    }

    public record AudioDuplicateFile(
        Path path,
        AudioMetadata metadata,
        AudioQualityAnalysis analysis
    ) {}
}
