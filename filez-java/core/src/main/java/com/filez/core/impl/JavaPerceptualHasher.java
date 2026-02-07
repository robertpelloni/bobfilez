package com.filez.core.impl;

import com.filez.core.interfaces.PerceptualHasher;
import com.filez.core.types.PerceptualHash;

import javax.imageio.ImageIO;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferByte;
import java.io.IOException;
import java.nio.file.Path;
import java.util.Optional;

public class JavaPerceptualHasher implements PerceptualHasher {

    private final String algorithm;

    public JavaPerceptualHasher(String algorithm) {
        this.algorithm = algorithm != null ? algorithm.toLowerCase() : "dhash";
    }

    public JavaPerceptualHasher() {
        this("dhash");
    }

    @Override
    public Optional<PerceptualHash> compute(Path path) {
        try {
            BufferedImage img = ImageIO.read(path.toFile());
            if (img == null) return Optional.empty();

            long hash = switch (algorithm) {
                case "dhash" -> computeDHash(img);
                case "ahash" -> computeAHash(img);
                default -> throw new IllegalArgumentException("Unsupported algorithm: " + algorithm);
            };

            return Optional.of(new PerceptualHash(hash, algorithm));
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    @Override
    public String method() {
        return algorithm;
    }

    private long computeDHash(BufferedImage img) {
        // Resize to 9x8 for dHash (comparing adjacent pixels)
        BufferedImage resized = resize(img, 9, 8);
        return computeDifferenceHash(resized);
    }

    private long computeAHash(BufferedImage img) {
        // Resize to 8x8 for aHash
        BufferedImage resized = resize(img, 8, 8);
        return computeAverageHash(resized);
    }

    private BufferedImage resize(BufferedImage img, int width, int height) {
        BufferedImage resized = new BufferedImage(width, height, BufferedImage.TYPE_BYTE_GRAY);
        Graphics2D g = resized.createGraphics();
        g.drawImage(img, 0, 0, width, height, null);
        g.dispose();
        return resized;
    }

    private long computeDifferenceHash(BufferedImage img) {
        long hash = 0;
        // DataBufferByte is faster if accessible, otherwise getRGB
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                int left = getGray(img, x, y);
                int right = getGray(img, x + 1, y);
                if (left > right) {
                    hash |= (1L << (y * 8 + x));
                }
            }
        }
        return hash;
    }

    private long computeAverageHash(BufferedImage img) {
        long sum = 0;
        int[] pixels = new int[64];
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                int val = getGray(img, x, y);
                pixels[y * 8 + x] = val;
                sum += val;
            }
        }
        double avg = sum / 64.0;
        long hash = 0;
        for (int i = 0; i < 64; i++) {
            if (pixels[i] > avg) {
                hash |= (1L << i);
            }
        }
        return hash;
    }

    private int getGray(BufferedImage img, int x, int y) {
        int rgb = img.getRGB(x, y);
        // Extract gray component (it's grayscale, so R=G=B)
        return rgb & 0xFF;
    }
}
