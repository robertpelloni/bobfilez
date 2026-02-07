package com.filez.core.impl;

import com.drew.imaging.ImageMetadataReader;
import com.drew.metadata.Metadata;
import com.drew.metadata.Directory;
import com.drew.metadata.exif.ExifIFD0Directory;
import com.drew.metadata.exif.ExifSubIFDDirectory;
import com.drew.metadata.exif.GpsDirectory;
import com.drew.metadata.jpeg.JpegDirectory;
import com.filez.core.interfaces.MetadataProvider;
import com.filez.core.types.ImageMetadata;

import java.io.IOException;
import java.nio.file.Path;
import java.time.Instant;
import java.time.ZoneId;
import java.util.Date;
import java.util.Optional;
import java.util.TimeZone;

public class JavaMetadataProvider implements MetadataProvider {

    @Override
    public Optional<ImageMetadata> read(Path path) {
        try {
            Metadata metadata = ImageMetadataReader.readMetadata(path.toFile());
            
            Optional<Instant> date = extractDate(metadata);
            Optional<String> make = extractString(metadata, ExifIFD0Directory.class, ExifIFD0Directory.TAG_MAKE);
            Optional<String> model = extractString(metadata, ExifIFD0Directory.class, ExifIFD0Directory.TAG_MODEL);
            
            int width = 0;
            int height = 0;
            
            // Try JPEG directory first
            JpegDirectory jpegDir = metadata.getFirstDirectoryOfType(JpegDirectory.class);
            if (jpegDir != null) {
                width = jpegDir.getImageWidth();
                height = jpegDir.getImageHeight();
            } else {
                // Fallback to EXIF
                ExifSubIFDDirectory exifDir = metadata.getFirstDirectoryOfType(ExifSubIFDDirectory.class);
                if (exifDir != null) {
                    // This might be 0 if not present
                    width = exifDir.getInt(ExifSubIFDDirectory.TAG_EXIF_IMAGE_WIDTH); 
                    height = exifDir.getInt(ExifSubIFDDirectory.TAG_EXIF_IMAGE_HEIGHT);
                }
            }

            Optional<Double> lat = Optional.empty();
            Optional<Double> lon = Optional.empty();
            
            GpsDirectory gpsDir = metadata.getFirstDirectoryOfType(GpsDirectory.class);
            if (gpsDir != null && gpsDir.getGeoLocation() != null) {
                lat = Optional.of(gpsDir.getGeoLocation().getLatitude());
                lon = Optional.of(gpsDir.getGeoLocation().getLongitude());
            }

            return Optional.of(new ImageMetadata(
                date,
                Optional.of(width).filter(w -> w > 0),
                Optional.of(height).filter(h -> h > 0),
                make,
                model,
                lat,
                lon
            ));

        } catch (IOException | com.drew.imaging.ImageProcessingException e) {
            // Log error if verbose or return empty
            return Optional.empty();
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    @Override
    public boolean canHandle(Path path) {
        String name = path.getFileName().toString().toLowerCase();
        return name.endsWith(".jpg") || name.endsWith(".jpeg") || name.endsWith(".png") ||
               name.endsWith(".tif") || name.endsWith(".tiff") || name.endsWith(".gif") ||
               name.endsWith(".bmp") || name.endsWith(".heic") || name.endsWith(".webp") ||
               name.endsWith(".cr2") || name.endsWith(".nef") || name.endsWith(".arw");
    }

    private Optional<Instant> extractDate(Metadata metadata) {
        // Try ExifSubIFD (Original Date)
        ExifSubIFDDirectory subIfd = metadata.getFirstDirectoryOfType(ExifSubIFDDirectory.class);
        if (subIfd != null) {
            Date date = subIfd.getDate(ExifSubIFDDirectory.TAG_DATETIME_ORIGINAL, TimeZone.getDefault());
            if (date != null) return Optional.of(date.toInstant());
            
            date = subIfd.getDate(ExifSubIFDDirectory.TAG_DATETIME_DIGITIZED, TimeZone.getDefault());
            if (date != null) return Optional.of(date.toInstant());
        }
        
        // Try ExifIFD0
        ExifIFD0Directory ifd0 = metadata.getFirstDirectoryOfType(ExifIFD0Directory.class);
        if (ifd0 != null) {
            Date date = ifd0.getDate(ExifIFD0Directory.TAG_DATETIME, TimeZone.getDefault());
            if (date != null) return Optional.of(date.toInstant());
        }
        
        return Optional.empty();
    }

    private <T extends Directory> Optional<String> extractString(Metadata metadata, Class<T> dirClass, int tagType) {
        T dir = metadata.getFirstDirectoryOfType(dirClass);
        if (dir != null) {
            String val = dir.getString(tagType);
            if (val != null && !val.isBlank()) {
                return Optional.of(val.trim());
            }
        }
        return Optional.empty();
    }
}
