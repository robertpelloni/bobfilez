package com.filez.core.engine;

import com.filez.core.interfaces.*;
import com.filez.core.registry.Registry;
import com.filez.core.types.*;

import java.nio.file.Path;
import java.util.List;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class Engine implements AutoCloseable {
    
    private final EngineConfig config;
    private final FileScanner scanner;
    private final Hasher hasher;
    private final MetadataProvider metadataProvider;
    private final AudioFingerprinter audioFingerprinter;
    private final AudioAnalyzer audioAnalyzer;
    private final ExecutorService executor;
    
    private Engine(EngineConfig config) {
        this.config = config;
        this.scanner = Registry.of(FileScanner.class).getOrThrow(config.scannerName());
        this.hasher = Registry.of(Hasher.class).getOrThrow(config.hasherName());
        this.metadataProvider = Registry.of(MetadataProvider.class).get(config.metadataProviderName()).orElse(null);
        this.audioFingerprinter = Registry.of(AudioFingerprinter.class).get(config.audioFingerprinterName()).orElse(null);
        this.audioAnalyzer = Registry.of(AudioAnalyzer.class).get(config.audioAnalyzerName()).orElse(null);
        this.executor = Executors.newFixedThreadPool(config.threadPoolSize());
    }
    
    public static Engine create(EngineConfig config) {
        return new Engine(config);
    }
    
    public static Engine createDefault() {
        return new Engine(EngineConfig.builder().build());
    }
    
    public List<FileInfo> scan(Path root) {
        return scanner.scan(List.of(root), Set.of(), config.followSymlinks());
    }
    
    public List<FileInfo> scan(List<Path> roots, Set<String> extensions) {
        return scanner.scan(roots, extensions, config.followSymlinks());
    }
    
    public List<FileInfo> scanImages(Path root) {
        return scanner.scan(List.of(root), config.imageExtensions(), config.followSymlinks());
    }
    
    public List<FileInfo> scanAudio(Path root) {
        return scanner.scan(List.of(root), config.audioExtensions(), config.followSymlinks());
    }
    
    public Hashes hash(Path path) {
        return hasher.hash(path);
    }
    
    public String fastHash(Path path) {
        return hasher.fast64(path);
    }
    
    public Optional<ImageMetadata> readImageMetadata(Path path) {
        return metadataProvider != null ? metadataProvider.read(path) : Optional.empty();
    }
    
    public Optional<AudioFingerprint> computeAudioFingerprint(Path path) {
        return audioFingerprinter != null ? audioFingerprinter.compute(path) : Optional.empty();
    }
    
    public Optional<AudioQualityAnalysis> analyzeAudio(Path path) {
        return audioAnalyzer != null ? audioAnalyzer.analyze(path) : Optional.empty();
    }
    
    public List<DuplicateGroup> findDuplicates(List<FileInfo> files) {
        DuplicateFinder finder = Registry.of(DuplicateFinder.class).getOrThrow("default");
        return finder.findDuplicates(files);
    }
    
    public List<AudioDuplicateGroup> findAudioDuplicates(List<FileInfo> files, AudioDuplicateFinder.AudioCompareOptions options) {
        AudioDuplicateFinder finder = Registry.of(AudioDuplicateFinder.class).getOrThrow("default");
        return finder.findDuplicates(files, options);
    }
    
    public EngineConfig config() {
        return config;
    }
    
    public ExecutorService executor() {
        return executor;
    }
    
    @Override
    public void close() {
        executor.shutdown();
    }
}
