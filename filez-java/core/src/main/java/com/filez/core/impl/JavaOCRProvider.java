package com.filez.core.impl;

import com.filez.core.interfaces.OCRProvider;
import com.filez.core.types.OCRResult;
import net.sourceforge.tess4j.Tesseract;
import net.sourceforge.tess4j.TesseractException;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Optional;

public class JavaOCRProvider implements OCRProvider {

    private final Tesseract tesseract;
    private final boolean available;

    public JavaOCRProvider() {
        this.tesseract = new Tesseract();
        // Try to find tessdata in standard locations or env var
        String tessDataPrefix = System.getenv("TESSDATA_PREFIX");
        if (tessDataPrefix != null) {
            tesseract.setDatapath(tessDataPrefix);
        } else if (Files.exists(Path.of("tessdata"))) {
            tesseract.setDatapath("tessdata");
        }
        
        // Basic check if Tesseract is likely to work (e.g. check for native lib load if possible)
        // Tess4J loads native libs lazily, so we assume available if class loaded
        this.available = true;
    }

    @Override
    public Optional<OCRResult> recognize(Path path, String language) {
        if (!available) return Optional.empty();

        try {
            tesseract.setLanguage(language);
            String text = tesseract.doOCR(path.toFile());
            
            // Tess4J doesn't give per-result confidence easily in simple doOCR
            // We'd need to use getWords() or result iterator for that.
            // For now, we estimate confidence based on text length/quality or just return 1.0 if successful.
            // Or we can use tesseract.getMeanConfidence() if we process via ResultIterator logic, but that's complex.
            // Let's assume 0.8 for successful recognition for now.
            
            return Optional.of(new OCRResult(text.trim(), 0.8f, language));
        } catch (TesseractException e) {
            // e.printStackTrace(); // Log if verbose?
            return Optional.empty();
        } catch (UnsatisfiedLinkError e) {
            System.err.println("OCR Error: Native library not found. Install Tesseract or check TESSDATA_PREFIX.");
            return Optional.empty();
        }
    }

    @Override
    public boolean isAvailable() {
        return available;
    }

    @Override
    public String[] supportedLanguages() {
        // This is hard to query without listing tessdata directory.
        // Return common ones.
        return new String[]{"eng", "deu", "fra", "ita", "spa", "por", "nld"};
    }
}
