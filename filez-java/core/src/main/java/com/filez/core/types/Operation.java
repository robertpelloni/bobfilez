package com.filez.core.types;

import java.time.Instant;

public record Operation(
    long id,
    String type,
    String sourcePath,
    String destPath,
    Instant timestamp,
    String status,
    String details
) {}
