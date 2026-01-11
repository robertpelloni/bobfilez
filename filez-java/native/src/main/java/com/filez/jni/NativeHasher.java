package com.filez.jni;

import com.filez.core.interfaces.Hasher;
import com.filez.core.impl.JavaHasher;

import java.nio.file.Path;
import java.util.Optional;

public class NativeHasher implements Hasher {
    
    private static final boolean BLAKE3_AVAILABLE = Blake3Jni.isAvailable();
    private static final boolean XXHASH_AVAILABLE = XxHash64Jni.isAvailable();
    private final JavaHasher fallback = new JavaHasher();
    
    @Override
    public String fast64(Path path) {
        if (XXHASH_AVAILABLE) {
            return XxHash64Jni.hashFileHex(path);
        }
        return fallback.fast64(path);
    }
    
    @Override
    public Optional<String> strong(Path path) {
        if (BLAKE3_AVAILABLE) {
            return Optional.of(Blake3Jni.hashFile(path));
        }
        return fallback.strong(path);
    }
    
    public boolean isNativeFastAvailable() {
        return XXHASH_AVAILABLE;
    }
    
    public boolean isNativeStrongAvailable() {
        return BLAKE3_AVAILABLE;
    }
    
    public static String getStatus() {
        return String.format("XXHash64: %s, BLAKE3: %s",
            XXHASH_AVAILABLE ? "native" : "fallback (CRC32)",
            BLAKE3_AVAILABLE ? "native" : "fallback (SHA-256)");
    }
}
