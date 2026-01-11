# Music Similarity Feature Specification

Based on analysis of [Similarity App](https://www.similarityapp.com/) and audio fingerprinting research.

## Overview

This document specifies the music similarity and quality analysis features to implement in filez-java, enabling detection of duplicate audio files and quality analysis of music collections.

---

## 1. Audio Comparison Algorithms

### 1.1 Tag-Based Comparison
Compares audio file metadata tags.

**Tags to Compare:**
- Artist
- Title  
- Album
- Genre
- Year
- Track number

**Algorithm:** Fuzzy string matching (Levenshtein distance, Jaro-Winkler)

**Threshold:** Default 80% similarity

**Use Case:** Quick pre-filter before content comparison

### 1.2 Content-Based Comparison (Acoustic Fingerprint)
Uses Chromaprint library for audio fingerprinting.

**Process:**
1. Decode audio to PCM (any format → 11025 Hz mono)
2. Generate STFT spectrogram
3. Extract chroma features (12 bins per frame)
4. Apply 16 filters to sliding 16x12 windows
5. Gray code encode to 32-bit fingerprints
6. Compare using Hamming distance

**Library:** Chromaprint (C/C++ via JNI)

**Threshold:** Default 80% similarity

**Performance:** ~30 bytes/second fingerprint size

### 1.3 Precise Algorithm
Enhanced fingerprinting with better accuracy for near-identical detection.

**Improvements over content-based:**
- Higher resolution spectral analysis
- More robust to encoding differences
- Better handling of slight timing variations

**Threshold:** Default 85% similarity (higher precision)

**Use Case:** Large collections (100K+ files), final verification

### 1.4 Fourth Algorithm (Large Collection Optimization)
Optimized for scanning huge collections with reduced memory.

**Strategy:**
- Fast hash pre-filtering by file size
- Duration-based bucketing (±3 seconds)
- Sparse fingerprint comparison
- Only compare across folder groups

---

## 2. Audio Quality Analysis

### 2.1 Quality Metrics

| Metric | Description | Bad Threshold |
|--------|-------------|---------------|
| Sample Rate | Samples per second | ≤22050 Hz |
| Bitrate | Bits per second | ≤64000 bps |
| Channels | Mono/Stereo | Mono |
| Clipping | Amplitude distortion count | ≥100 samples |
| Mean(Abs) | Average amplitude (0-1) | ≥0.1 |
| Step | Min amplitude step (bit depth) | ≥0.00004 |
| Max(Abs) | Peak amplitude (0-1) | ≤0.25 |
| Max Freq | Highest frequency present | ≤12000 Hz |
| Clicks | Vinyl distortion indicator | N/A |

### 2.2 Quality Rating Formula

```java
double calculateRating(AudioQualityAnalysis a) {
    double rating = 1.0;
    
    // Sample rate (22050-44100 Hz range)
    rating *= 0.5 + 0.5 * (clamp(a.sampleRate, 22050, 44100) - 22050) / 22050;
    
    // Bitrate (64000-320000 range)
    if (a.bitrate > 0) {
        rating *= 0.5 + 0.5 * (clamp(a.bitrate, 64000, 320000) - 64000) / 256000;
    }
    
    // Channels penalty
    if (a.channels == 1) rating *= 0.8;
    
    // Clipping penalty
    if (a.clipping >= 100) rating *= 0.7;
    
    // Dynamic range penalty
    if (a.absMax <= 0.25) rating *= 0.8;
    
    // High frequency cutoff penalty (lossy artifacts)
    if (a.maxFreq <= 12000) rating *= 0.85;
    
    return rating;
}
```

### 2.3 Spectrum Analysis
FFT-based frequency analysis with configurable parameters:

- **FFT Size:** 512, 1024, 2048, 4096 (default: 2048)
- **Overlap:** 0%, 50%, 75%, 90%
- **Window:** Hann (default), Hamming, Blackman
- **Output:** dB scale, logarithmic frequency axis

### 2.4 Sonogram (Spectrogram)
Time-frequency visualization:

- **Time Resolution:** hop size / sample rate
- **Frequency Resolution:** sample rate / FFT size
- **Color Map:** Amplitude → color gradient

---

## 3. Supported Audio Formats

### Native Support (via FFmpeg/JNI)
- MP3 (MPEG-1/2/2.5 Layer 3)
- M4A/AAC (MPEG-4 Audio)
- WMA (Windows Media Audio)
- OGG/OGA (Vorbis)
- FLAC (Free Lossless Audio Codec)
- APE (Monkey's Audio)
- WV (WavPack)
- MPC (MusePack)
- OPUS
- TTA (True Audio)
- WAV/AIFF (PCM)
- ALAC (Apple Lossless)

### Tag Format Support
- ID3v1, ID3v2.3, ID3v2.4
- APEv2
- Vorbis Comments
- WMA Attributes

---

## 4. Implementation Architecture

### 4.1 Java Interfaces

```java
// Audio fingerprinting
interface AudioFingerprinter {
    Optional<AudioFingerprint> compute(Path path);
    String algorithm();
    double compare(AudioFingerprint a, AudioFingerprint b);
    double defaultThreshold();
}

// Quality analysis
interface AudioAnalyzer {
    Optional<AudioQualityAnalysis> analyze(Path path);
    boolean canHandle(Path path);
    double calculateRating(AudioQualityAnalysis analysis);
}

// Duplicate finding
interface AudioDuplicateFinder {
    List<AudioDuplicateGroup> findDuplicates(List<FileInfo> files, AudioCompareOptions options);
}

// Metadata provider
interface AudioMetadataProvider {
    Optional<AudioMetadata> read(Path path);
    boolean canHandle(Path path);
    boolean writeTags(Path path, AudioMetadata metadata);
}
```

### 4.2 JNI Components

| Component | Language | Library |
|-----------|----------|---------|
| Audio decoding | C | FFmpeg libavcodec |
| Fingerprinting | C++ | Chromaprint |
| FFT | C | FFmpeg/KissFFT |
| Tag reading | Java | metadata-extractor or JAudioTagger |

### 4.3 Data Types

```java
record AudioFingerprint(byte[] data, String algorithm, int duration);

record AudioQualityAnalysis(
    int sampleRate, int bitrate, int channels, long durationMs,
    String format, String dataType,
    int clipping, int silence,
    double absMean, double minStep, double absMax,
    int maxFreq, int clicks, double rating
);

record AudioMetadata(
    String format, int bitrate, int sampleRate, int channels, long durationMs,
    Optional<String> title, Optional<String> artist, Optional<String> album,
    Optional<String> genre, Optional<Integer> year, Optional<Integer> trackNumber
);

record AudioDuplicateGroup(
    List<AudioDuplicateFile> files,
    double tagScore, double contentScore, double preciseScore
);
```

---

## 5. Comparison Options

### 5.1 AudioCompareOptions

```java
record AudioCompareOptions(
    boolean useTags,           // Enable tag comparison
    boolean useContent,        // Enable content fingerprint
    boolean usePrecise,        // Enable precise algorithm
    double tagThreshold,       // 0.0-1.0 (default: 0.80)
    double contentThreshold,   // 0.0-1.0 (default: 0.80)
    double preciseThreshold,   // 0.0-1.0 (default: 0.85)
    int durationToleranceMs,   // Duration difference tolerance (default: 3000)
    int folderGroup1,          // Folder group filter (-1 = all)
    int folderGroup2           // Compare only across groups
);
```

### 5.2 Presets

| Preset | Tags | Content | Precise | Thresholds |
|--------|------|---------|---------|------------|
| Default | ✓ | ✓ | ✓ | 80/80/85 |
| Precise Only | ✗ | ✗ | ✓ | -/-/90 |
| Large Collection | ✗ | ✗ | ✓ | -/-/85 |
| Quick Scan | ✓ | ✗ | ✗ | 90/-/- |

---

## 6. CLI Commands

### scan-audio
```bash
fo scan-audio <paths...> [--extensions mp3,flac,...] [--recursive]
```

### audio-duplicates
```bash
fo audio-duplicates <paths...> 
    [--algorithm tags|content|precise|all]
    [--threshold 0.85]
    [--group1 <folder>] [--group2 <folder>]
    [--output json|table]
```

### audio-analyze
```bash
fo audio-analyze <paths...>
    [--sort rating|bitrate|samplerate|clipping]
    [--filter "rating<0.5"]
    [--output json|table]
```

### audio-spectrum
```bash
fo audio-spectrum <file>
    [--fft-size 2048]
    [--output png|json]
```

---

## 7. Performance Targets

| Operation | Target |
|-----------|--------|
| Fingerprint generation | <0.1x real-time |
| Fingerprint comparison | <1μs per pair |
| Quality analysis | <0.2x real-time |
| 10K file scan | <5 minutes |
| 100K file scan | <1 hour |
| Memory (64-bit) | <4GB for 100K files |

---

## 8. Libraries & Dependencies

### Java Dependencies
```kotlin
// build.gradle.kts
dependencies {
    // Audio metadata
    implementation("net.jthink:jaudiotagger:3.0.1")
    
    // FFT (pure Java fallback)
    implementation("edu.emory.mathcs:JTransforms:3.1")
    
    // Audio processing
    implementation("be.tarsos.dsp:TarsosDSP:2.5")
}
```

### Native Dependencies (JNI)
- Chromaprint 1.6.0+
- FFmpeg libavcodec, libavformat, libavutil, libswresample
- KissFFT (if not using FFmpeg FFT)

---

## 9. References

- [Chromaprint](https://github.com/acoustid/chromaprint) - Acoustic fingerprinting
- [AcoustID](https://acoustid.org/) - Music identification service
- [audfprint](https://github.com/dpwe/audfprint) - Landmark-based fingerprinting
- [TarsosDSP](https://github.com/JorenSix/TarsosDSP) - Java audio processing
- [JAudioTagger](https://github.com/java-native/jaudiotagger) - Audio metadata
- [JTransforms](https://github.com/wendykierp/JTransforms) - Pure Java FFT
