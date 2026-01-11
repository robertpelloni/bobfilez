package com.filez.core.interfaces;

import com.filez.core.types.ImageMetadata;
import java.nio.file.Path;
import java.util.Optional;

public interface MetadataProvider {
    
    Optional<ImageMetadata> read(Path path);
    
    boolean canHandle(Path path);
}
