package com.filez.core.types;

public record AudioFingerprint(
    byte[] data,
    String algorithm,
    int duration
) {
    public static final String ALGORITHM_CHROMAPRINT = "chromaprint";
    public static final String ALGORITHM_CONTENT = "content";
    public static final String ALGORITHM_PRECISE = "precise";

    public double similarity(AudioFingerprint other) {
        if (!this.algorithm.equals(other.algorithm)) {
            throw new IllegalArgumentException(
                "Cannot compare fingerprints from different algorithms: " + 
                this.algorithm + " vs " + other.algorithm
            );
        }
        
        if (this.data.length != other.data.length) {
            return 0.0;
        }
        
        int matchingBits = 0;
        int totalBits = this.data.length * 8;
        
        for (int i = 0; i < this.data.length; i++) {
            int xor = (this.data[i] ^ other.data[i]) & 0xFF;
            matchingBits += 8 - Integer.bitCount(xor);
        }
        
        return (double) matchingBits / totalBits;
    }

    public boolean isSimilar(AudioFingerprint other, double threshold) {
        return similarity(other) >= threshold;
    }
}
