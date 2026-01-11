package com.filez.jni;

import com.filez.core.interfaces.Hasher;
import com.filez.core.registry.Registry;

public final class NativeModule {
    
    private NativeModule() {}
    
    public static void register() {
        Registry<Hasher> hasherRegistry = Registry.of(Hasher.class);
        
        if (Blake3Jni.isAvailable() || XxHash64Jni.isAvailable()) {
            hasherRegistry.register("native", NativeHasher::new);
        }
    }
    
    public static String getStatus() {
        StringBuilder sb = new StringBuilder("Native Module Status:\n");
        sb.append("  XXHash64: ").append(XxHash64Jni.isAvailable() ? "loaded" : "not available");
        if (!XxHash64Jni.isAvailable() && XxHash64Jni.getLoadError() != null) {
            sb.append(" (").append(XxHash64Jni.getLoadError().getMessage()).append(")");
        }
        sb.append("\n");
        
        sb.append("  BLAKE3: ").append(Blake3Jni.isAvailable() ? "loaded" : "not available");
        if (!Blake3Jni.isAvailable() && Blake3Jni.getLoadError() != null) {
            sb.append(" (").append(Blake3Jni.getLoadError().getMessage()).append(")");
        }
        
        return sb.toString();
    }
}
