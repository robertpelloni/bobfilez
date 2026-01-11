package com.filez.core.interfaces;

import com.filez.core.types.Hashes;
import java.nio.file.Path;
import java.util.Optional;

public interface Hasher {
    
    String fast64(Path path);
    
    Optional<String> strong(Path path);
    
    default Hashes hash(Path path) {
        return new Hashes(fast64(path), strong(path));
    }
    
    default Hashes hashFastOnly(Path path) {
        return Hashes.ofFast(fast64(path));
    }
}
