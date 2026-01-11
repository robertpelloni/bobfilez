package com.filez.core.types;

/**
 * Perceptual hash for image similarity comparison.
 */
public record PerceptualHash(
    long value,
    String method
) {
    public PerceptualHash {
        if (method == null || method.isBlank()) {
            throw new IllegalArgumentException("method must not be null or blank");
        }
    }

    /**
     * Computes the Hamming distance between this hash and another.
     * Lower distance means more similar images.
     *
     * @param other The hash to compare with
     * @return Hamming distance (0 = identical, 64 = completely different for 64-bit hash)
     */
    public int hammingDistance(PerceptualHash other) {
        if (!this.method.equals(other.method)) {
            throw new IllegalArgumentException(
                "Cannot compare hashes from different methods: " + this.method + " vs " + other.method
            );
        }
        return Long.bitCount(this.value ^ other.value);
    }

    /**
     * Returns the similarity score (0.0 to 1.0) based on Hamming distance.
     * 1.0 = identical, 0.0 = completely different
     */
    public double similarity(PerceptualHash other) {
        int distance = hammingDistance(other);
        return 1.0 - (distance / 64.0);
    }

    /**
     * Returns true if images are considered similar (distance <= threshold).
     */
    public boolean isSimilar(PerceptualHash other, int threshold) {
        return hammingDistance(other) <= threshold;
    }

    /**
     * Common method names.
     */
    public static final String METHOD_DHASH = "dhash";
    public static final String METHOD_PHASH = "phash";
    public static final String METHOD_AHASH = "ahash";
}
