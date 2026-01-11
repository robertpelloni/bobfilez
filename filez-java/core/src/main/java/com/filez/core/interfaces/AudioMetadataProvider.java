package com.filez.core.interfaces;

import com.filez.core.types.AudioMetadata;
import java.nio.file.Path;
import java.util.Optional;

public interface AudioMetadataProvider {
    
    Optional<AudioMetadata> read(Path path);
    
    boolean canHandle(Path path);
    
    boolean writeTags(Path path, AudioMetadata metadata);
}
