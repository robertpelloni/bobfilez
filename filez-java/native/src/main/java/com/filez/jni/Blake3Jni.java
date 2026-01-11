package com.filez.jni;

import java.nio.file.Path;

public final class Blake3Jni {
    
    private static final String LIBRARY_NAME = "filez_native";
    private static volatile boolean loaded = false;
    private static volatile Throwable loadError = null;
    
    static {
        tryLoad();
    }
    
    private Blake3Jni() {}
    
    public static synchronized boolean loadLibrary() {
        if (loaded) return true;
        if (loadError != null) return false;
        return tryLoad();
    }
    
    private static boolean tryLoad() {
        try {
            System.loadLibrary(LIBRARY_NAME);
            loaded = true;
            return true;
        } catch (UnsatisfiedLinkError e) {
            loadError = e;
            return false;
        }
    }
    
    public static boolean isAvailable() {
        return loaded;
    }
    
    public static Throwable getLoadError() {
        return loadError;
    }
    
    public static String hashFile(Path path) {
        if (!loaded) {
            throw new IllegalStateException("Native library not loaded");
        }
        return nativeHashFile(path.toAbsolutePath().toString());
    }
    
    public static String hashBytes(byte[] data) {
        if (!loaded) {
            throw new IllegalStateException("Native library not loaded");
        }
        return nativeHashBytes(data);
    }
    
    public static String hashFileStreaming(Path path, int bufferSize) {
        if (!loaded) {
            throw new IllegalStateException("Native library not loaded");
        }
        int size = bufferSize <= 0 ? 65536 : bufferSize;
        return nativeHashFileStreaming(path.toAbsolutePath().toString(), size);
    }
    
    private static native String nativeHashFile(String path);
    private static native String nativeHashBytes(byte[] data);
    private static native String nativeHashFileStreaming(String path, int bufferSize);
}
