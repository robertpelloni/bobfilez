package com.filez.core.types;

public record AudioQualityAnalysis(
    int sampleRate,
    int bitrate,
    int channels,
    long durationMs,
    String format,
    String dataType,
    int clipping,
    int silence,
    double absMean,
    double minStep,
    double absMax,
    int maxFreq,
    int clicks,
    double rating
) {
    public boolean hasClipping() {
        return clipping >= 100;
    }

    public boolean hasLowBitrate() {
        return bitrate > 0 && bitrate <= 64000;
    }

    public boolean hasLowSampleRate() {
        return sampleRate <= 22050;
    }

    public boolean hasFrequencyCutoff() {
        return maxFreq > 0 && maxFreq <= 12000;
    }

    public boolean isMono() {
        return channels == 1;
    }

    public boolean hasLowDynamicRange() {
        return absMax <= 0.25;
    }

    public static Builder builder() {
        return new Builder();
    }

    public static class Builder {
        private int sampleRate;
        private int bitrate;
        private int channels = 2;
        private long durationMs;
        private String format = "unknown";
        private String dataType = "16";
        private int clipping;
        private int silence;
        private double absMean;
        private double minStep = 0.000030517578125;
        private double absMax = 1.0;
        private int maxFreq = 20000;
        private int clicks;
        private double rating = 1.0;

        public Builder sampleRate(int v) { sampleRate = v; return this; }
        public Builder bitrate(int v) { bitrate = v; return this; }
        public Builder channels(int v) { channels = v; return this; }
        public Builder durationMs(long v) { durationMs = v; return this; }
        public Builder format(String v) { format = v; return this; }
        public Builder dataType(String v) { dataType = v; return this; }
        public Builder clipping(int v) { clipping = v; return this; }
        public Builder silence(int v) { silence = v; return this; }
        public Builder absMean(double v) { absMean = v; return this; }
        public Builder minStep(double v) { minStep = v; return this; }
        public Builder absMax(double v) { absMax = v; return this; }
        public Builder maxFreq(int v) { maxFreq = v; return this; }
        public Builder clicks(int v) { clicks = v; return this; }
        public Builder rating(double v) { rating = v; return this; }

        public AudioQualityAnalysis build() {
            return new AudioQualityAnalysis(
                sampleRate, bitrate, channels, durationMs, format, dataType,
                clipping, silence, absMean, minStep, absMax, maxFreq, clicks, rating
            );
        }
    }
}
