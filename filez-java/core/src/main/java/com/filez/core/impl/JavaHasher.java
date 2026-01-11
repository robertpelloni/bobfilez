package com.filez.core.impl;

import com.filez.core.interfaces.Hasher;

import java.io.IOException;
import java.io.InputStream;
import java.io.UncheckedIOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HexFormat;
import java.util.Optional;
import java.util.zip.CRC32;

public class JavaHasher implements Hasher {

    private static final int BUFFER_SIZE = 64 * 1024;
    private static final HexFormat HEX = HexFormat.of();

    @Override
    public String fast64(Path path) {
        CRC32 crc = new CRC32();
        byte[] buffer = new byte[BUFFER_SIZE];
        
        try (InputStream in = Files.newInputStream(path)) {
            int read;
            while ((read = in.read(buffer)) != -1) {
                crc.update(buffer, 0, read);
            }
        } catch (IOException e) {
            throw new UncheckedIOException("Failed to hash file: " + path, e);
        }
        
        return Long.toHexString(crc.getValue());
    }

    @Override
    public Optional<String> strong(Path path) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] buffer = new byte[BUFFER_SIZE];
            
            try (InputStream in = Files.newInputStream(path)) {
                int read;
                while ((read = in.read(buffer)) != -1) {
                    md.update(buffer, 0, read);
                }
            }
            
            return Optional.of(HEX.formatHex(md.digest()));
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 not available", e);
        } catch (IOException e) {
            throw new UncheckedIOException("Failed to hash file: " + path, e);
        }
    }
}
