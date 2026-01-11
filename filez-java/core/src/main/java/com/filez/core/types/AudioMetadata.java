package com.filez.core.types;

import java.util.Optional;

public record AudioMetadata(
    String format,
    int bitrate,
    int sampleRate,
    int channels,
    long durationMs,
    Optional<String> title,
    Optional<String> artist,
    Optional<String> album,
    Optional<String> genre,
    Optional<Integer> year,
    Optional<Integer> trackNumber
) {
    public static AudioMetadata empty() {
        return new AudioMetadata(
            "unknown", 0, 0, 0, 0,
            Optional.empty(), Optional.empty(), Optional.empty(),
            Optional.empty(), Optional.empty(), Optional.empty()
        );
    }

    public boolean isStereo() {
        return channels >= 2;
    }

    public double durationSeconds() {
        return durationMs / 1000.0;
    }
}
