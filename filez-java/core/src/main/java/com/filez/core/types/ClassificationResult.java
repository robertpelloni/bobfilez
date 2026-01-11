package com.filez.core.types;

/**
 * Result from AI image classification.
 */
public record ClassificationResult(
    String label,
    float confidence
) {
    public ClassificationResult {
        if (label == null || label.isBlank()) {
            throw new IllegalArgumentException("label must not be null or blank");
        }
        if (confidence < 0.0f || confidence > 1.0f) {
            throw new IllegalArgumentException("confidence must be between 0 and 1");
        }
    }

    /**
     * Returns true if this classification is confident enough.
     */
    public boolean isConfident(float threshold) {
        return confidence >= threshold;
    }
}
