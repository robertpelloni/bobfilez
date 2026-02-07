package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.core.interfaces.FileScanner;
import com.filez.core.interfaces.OCRProvider;
import com.filez.core.registry.Registry;
import com.filez.core.types.FileInfo;
import com.filez.core.types.OCRResult;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;
import picocli.CommandLine.ParentCommand;

import java.nio.file.Path;
import java.util.List;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.Callable;

@Command(
    name = "ocr",
    description = "Extract text from images using OCR"
)
public class OCRCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Parameters(index = "0..*", arity = "1..*", paramLabel = "PATH", description = "Files or directories")
    private List<Path> paths;

    @Option(names = {"-e", "--ext"}, split = ",", description = "Filter by extensions", 
            defaultValue = "jpg,jpeg,png,tiff,bmp,pdf")
    private Set<String> extensions;

    @Option(names = {"-l", "--lang"}, description = "Language code (eng, deu, etc.)", defaultValue = "eng")
    private String language;

    @Option(names = {"--provider"}, description = "OCR provider", defaultValue = "tesseract")
    private String providerName;

    @Option(names = {"--format"}, description = "Output format (text, json)", defaultValue = "text")
    private String format;

    @Override
    public Integer call() {
        FileScanner scanner = Registry.of(FileScanner.class).getOrThrow(parent.getScannerName());
        
        Optional<OCRProvider> providerOpt = Registry.of(OCRProvider.class).get(providerName);
        if (providerOpt.isEmpty()) {
            System.err.println("OCR provider not available: " + providerName);
            System.err.println("Available providers: " + Registry.of(OCRProvider.class).names());
            System.err.println("Note: OCR requires Tesseract to be installed and TESSDATA_PREFIX to be set.");
            return 1;
        }
        
        OCRProvider provider = providerOpt.get();
        List<FileInfo> files = scanner.scan(paths, extensions, false);

        if (parent.isVerbose()) {
            System.err.println("Running OCR on " + files.size() + " files...");
        }

        int processed = 0;
        int errors = 0;

        for (FileInfo file : files) {
            try {
                Optional<OCRResult> result = provider.recognize(file.path(), language);
                if (result.isPresent() && result.get().hasText()) {
                    if ("json".equals(format)) {
                        printJson(file.path(), result.get());
                    } else {
                        printText(file.path(), result.get());
                    }
                    processed++;
                }
            } catch (Exception e) {
                errors++;
                if (parent.isVerbose()) {
                    System.err.println("Error processing " + file.path() + ": " + e.getMessage());
                }
            }
        }

        System.err.printf("%nProcessed %d files (%d errors)%n", processed, errors);
        return errors > 0 && processed == 0 ? 1 : 0;
    }

    private void printText(Path path, OCRResult result) {
        System.out.println("=== " + path.getFileName() + " ===");
        System.out.println(result.text());
        System.out.println("Confidence: " + result.confidence());
        System.out.println();
    }

    private void printJson(Path path, OCRResult result) {
        System.out.printf("{\"path\":\"%s\",\"text\":\"%s\",\"confidence\":%.2f}%n",
            escapeJson(path.toString()), escapeJson(result.text()), result.confidence());
    }

    private String escapeJson(String s) {
        return s.replace("\\", "\\\\")
                .replace("\"", "\\\"")
                .replace("\n", "\\n")
                .replace("\r", "\\r")
                .replace("\t", "\\t");
    }
}
