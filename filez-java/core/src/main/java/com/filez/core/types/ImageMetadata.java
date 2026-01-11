package com.filez.core.types;

import java.util.Optional;

/**
 * Image metadata including dates and GPS coordinates.
 */
public record ImageMetadata(
    Optional<DateMetadata> date,
    boolean hasGps,
    double gpsLatitude,
    double gpsLongitude,
    Optional<Integer> width,
    Optional<Integer> height,
    Optional<String> cameraMake,
    Optional<String> cameraModel,
    Optional<Integer> orientation
) {
    /**
     * Creates an empty ImageMetadata.
     */
    public static ImageMetadata empty() {
        return new ImageMetadata(
            Optional.empty(),
            false, 0.0, 0.0,
            Optional.empty(),
            Optional.empty(),
            Optional.empty(),
            Optional.empty(),
            Optional.empty()
        );
    }

    /**
     * Builder for constructing ImageMetadata.
     */
    public static Builder builder() {
        return new Builder();
    }

    public static class Builder {
        private Optional<DateMetadata> date = Optional.empty();
        private boolean hasGps = false;
        private double gpsLatitude = 0.0;
        private double gpsLongitude = 0.0;
        private Optional<Integer> width = Optional.empty();
        private Optional<Integer> height = Optional.empty();
        private Optional<String> cameraMake = Optional.empty();
        private Optional<String> cameraModel = Optional.empty();
        private Optional<Integer> orientation = Optional.empty();

        public Builder date(DateMetadata date) {
            this.date = Optional.ofNullable(date);
            return this;
        }

        public Builder gps(double latitude, double longitude) {
            this.hasGps = true;
            this.gpsLatitude = latitude;
            this.gpsLongitude = longitude;
            return this;
        }

        public Builder dimensions(int width, int height) {
            this.width = Optional.of(width);
            this.height = Optional.of(height);
            return this;
        }

        public Builder camera(String make, String model) {
            this.cameraMake = Optional.ofNullable(make);
            this.cameraModel = Optional.ofNullable(model);
            return this;
        }

        public Builder orientation(int orientation) {
            this.orientation = Optional.of(orientation);
            return this;
        }

        public ImageMetadata build() {
            return new ImageMetadata(
                date, hasGps, gpsLatitude, gpsLongitude,
                width, height, cameraMake, cameraModel, orientation
            );
        }
    }
}
