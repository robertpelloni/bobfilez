package com.filez.core.types;

import java.time.LocalDate;
import java.util.Optional;

/**
 * Date metadata extracted from files (EXIF, etc.).
 */
public record DateMetadata(
    Optional<LocalDate> originalDate,    // Date photo was taken
    Optional<LocalDate> digitizedDate,   // Date digitized
    Optional<LocalDate> modifiedDate,    // Date modified in software
    String source                         // Source of the date (exif, xmp, filename, etc.)
) {
    public DateMetadata {
        if (source == null || source.isBlank()) {
            throw new IllegalArgumentException("source must not be null or blank");
        }
    }

    /**
     * Returns the best available date, preferring original > digitized > modified.
     */
    public Optional<LocalDate> bestDate() {
        return originalDate
            .or(() -> digitizedDate)
            .or(() -> modifiedDate);
    }

    /**
     * Creates a DateMetadata with only the original date.
     */
    public static DateMetadata ofOriginal(LocalDate date, String source) {
        return new DateMetadata(
            Optional.of(date),
            Optional.empty(),
            Optional.empty(),
            source
        );
    }
}
