package com.filez.core.types;

/**
 * Result from OCR text recognition.
 */
public record OCRResult(
    String text,
    float confidence,
    String language
) {
    public OCRResult {
        if (text == null) {
            throw new IllegalArgumentException("text must not be null");
        }
        if (confidence < 0.0f || confidence > 1.0f) {
            throw new IllegalArgumentException("confidence must be between 0 and 1");
        }
    }

    /**
     * Creates an empty OCR result (no text detected).
     */
    public static OCRResult empty(String language) {
        return new OCRResult("", 0.0f, language);
    }

    /**
     * Returns true if meaningful text was detected.
     */
    public boolean hasText() {
        return !text.isBlank() && confidence > 0.1f;
    }
}
