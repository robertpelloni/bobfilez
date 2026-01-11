package com.filez.jni;

import java.nio.file.Path;

public final class XxHash64Jni {
    
    private static final String LIBRARY_NAME = "filez_native";
    private static volatile boolean loaded = false;
    private static volatile Throwable loadError = null;
    
    static {
        tryLoad();
    }
    
    private XxHash64Jni() {}
    
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
    
    public static long hashFile(Path path) {
        if (!loaded) {
            throw new IllegalStateException("Native library not loaded");
        }
        return nativeHashFile(path.toAbsolutePath().toString());
    }
    
    public static long hashBytes(byte[] data) {
        if (!loaded) {
            throw new IllegalStateException("Native library not loaded");
        }
        return nativeHashBytes(data);
    }
    
    public static long hashFileWithSeed(Path path, long seed) {
        if (!loaded) {
            throw new IllegalStateException("Native library not loaded");
        }
        return nativeHashFileWithSeed(path.toAbsolutePath().toString(), seed);
    }
    
    public static String hashFileHex(Path path) {
        return String.format("%016x", hashFile(path));
    }
    
    public static String hashBytesHex(byte[] data) {
        return String.format("%016x", hashBytes(data));
    }
    
    private static native long nativeHashFile(String path);
    private static native long nativeHashBytes(byte[] data);
    private static native long nativeHashFileWithSeed(String path, long seed);
}
