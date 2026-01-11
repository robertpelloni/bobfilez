package com.filez.core.engine;

import java.nio.file.Path;
import java.util.Set;

public record EngineConfig(
    Path databasePath,
    String scannerName,
    String hasherName,
    String metadataProviderName,
    String audioFingerprinterName,
    String audioAnalyzerName,
    Set<String> imageExtensions,
    Set<String> audioExtensions,
    boolean followSymlinks,
    int threadPoolSize
) {
    public static final Set<String> DEFAULT_IMAGE_EXTENSIONS = Set.of(
        "jpg", "jpeg", "png", "gif", "bmp", "tiff", "tif", "webp", "heic", "heif", "raw", "cr2", "nef", "arw"
    );
    
    public static final Set<String> DEFAULT_AUDIO_EXTENSIONS = Set.of(
        "mp3", "m4a", "aac", "wma", "ogg", "wav", "flac", "ape", "wv", "mpc", "opus", "tta", "aiff", "alac"
    );
    
    public static Builder builder() {
        return new Builder();
    }
    
    public static class Builder {
        private Path databasePath = Path.of("filez.db");
        private String scannerName = "nio";
        private String hasherName = "default";
        private String metadataProviderName = "metadata-extractor";
        private String audioFingerprinterName = "chromaprint";
        private String audioAnalyzerName = "ffmpeg";
        private Set<String> imageExtensions = DEFAULT_IMAGE_EXTENSIONS;
        private Set<String> audioExtensions = DEFAULT_AUDIO_EXTENSIONS;
        private boolean followSymlinks = false;
        private int threadPoolSize = Runtime.getRuntime().availableProcessors();
        
        public Builder databasePath(Path p) { databasePath = p; return this; }
        public Builder scanner(String n) { scannerName = n; return this; }
        public Builder hasher(String n) { hasherName = n; return this; }
        public Builder metadataProvider(String n) { metadataProviderName = n; return this; }
        public Builder audioFingerprinter(String n) { audioFingerprinterName = n; return this; }
        public Builder audioAnalyzer(String n) { audioAnalyzerName = n; return this; }
        public Builder imageExtensions(Set<String> e) { imageExtensions = e; return this; }
        public Builder audioExtensions(Set<String> e) { audioExtensions = e; return this; }
        public Builder followSymlinks(boolean f) { followSymlinks = f; return this; }
        public Builder threadPoolSize(int s) { threadPoolSize = s; return this; }
        
        public EngineConfig build() {
            return new EngineConfig(
                databasePath, scannerName, hasherName, metadataProviderName,
                audioFingerprinterName, audioAnalyzerName,
                imageExtensions, audioExtensions, followSymlinks, threadPoolSize
            );
        }
    }
}
