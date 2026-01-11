package com.filez.core.types;

import java.util.Optional;

/**
 * Hash values for a file.
 * Contains both fast (for quick comparison) and strong (for verification) hashes.
 */
public record Hashes(
    String fast64,           // XXHash64 or similar fast hash
    Optional<String> strong  // BLAKE3 or SHA-256 for verification
) {
    public Hashes {
        if (fast64 == null || fast64.isBlank()) {
            throw new IllegalArgumentException("fast64 hash must not be null or blank");
        }
    }

    /**
     * Creates a Hashes with only the fast hash.
     */
    public static Hashes ofFast(String fast64) {
        return new Hashes(fast64, Optional.empty());
    }

    /**
     * Creates a Hashes with both fast and strong hashes.
     */
    public static Hashes of(String fast64, String strong) {
        return new Hashes(fast64, Optional.of(strong));
    }
}
