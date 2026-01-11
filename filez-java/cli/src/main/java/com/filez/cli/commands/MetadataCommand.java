package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.core.interfaces.FileScanner;
import com.filez.core.interfaces.MetadataProvider;
import com.filez.core.registry.Registry;
import com.filez.core.types.FileInfo;
import com.filez.core.types.ImageMetadata;
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
    name = "metadata",
    description = "Extract file metadata (EXIF, dates, etc.)"
)
public class MetadataCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Parameters(index = "0..*", arity = "1..*", paramLabel = "PATH", description = "Files or directories")
    private List<Path> paths;

    @Option(names = {"-e", "--ext"}, split = ",", description = "Filter by extensions", 
            defaultValue = "jpg,jpeg,png,tiff,heic,raw,cr2,nef,arw")
    private Set<String> extensions;

    @Option(names = {"--provider"}, description = "Metadata provider to use", defaultValue = "exiv2")
    private String providerName;

    @Option(names = {"--format"}, description = "Output format (text, json)", defaultValue = "text")
    private String format;

    @Override
    public Integer call() {
        FileScanner scanner = Registry.of(FileScanner.class).getOrThrow(parent.getScannerName());
        
        Optional<MetadataProvider> providerOpt = Registry.of(MetadataProvider.class).get(providerName);
        if (providerOpt.isEmpty()) {
            System.err.println("Metadata provider not available: " + providerName);
            System.err.println("Available providers: " + Registry.of(MetadataProvider.class).names());
            System.err.println("Note: Metadata extraction requires additional dependencies.");
            return 1;
        }
        
        MetadataProvider provider = providerOpt.get();
        List<FileInfo> files = scanner.scan(paths, extensions, false);

        if (parent.isVerbose()) {
            System.err.println("Extracting metadata from " + files.size() + " files...");
        }

        int processed = 0;
        int errors = 0;

        for (FileInfo file : files) {
            try {
                Optional<ImageMetadata> metadata = provider.read(file.path());
                if (metadata.isPresent()) {
                    if ("json".equals(format)) {
                        printJson(file.path(), metadata.get());
                    } else {
                        printText(file.path(), metadata.get());
                    }
                    processed++;
                }
            } catch (Exception e) {
                errors++;
                if (parent.isVerbose()) {
                    System.err.println("Error extracting metadata from " + file.path() + ": " + e.getMessage());
                }
            }
        }

        System.err.printf("%nProcessed %d files (%d errors)%n", processed, errors);
        return errors > 0 && processed == 0 ? 1 : 0;
    }

    private void printText(Path path, ImageMetadata meta) {
        System.out.println("=== " + path.getFileName() + " ===");
        meta.date().ifPresent(d -> System.out.println("  Date: " + d));
        meta.cameraMake().ifPresent(v -> System.out.println("  Camera Make: " + v));
        meta.cameraModel().ifPresent(v -> System.out.println("  Camera Model: " + v));
        meta.width().ifPresent(w -> meta.height().ifPresent(h -> 
            System.out.println("  Dimensions: " + w + "x" + h)));
        if (meta.hasGps()) {
            System.out.println("  GPS: " + meta.gpsLatitude() + ", " + meta.gpsLongitude());
        }
        System.out.println();
    }

    private void printJson(Path path, ImageMetadata meta) {
        StringBuilder sb = new StringBuilder();
        sb.append("{\"path\":\"").append(escapeJson(path.toString())).append("\"");
        meta.date().ifPresent(d -> sb.append(",\"date\":\"").append(d).append("\""));
        meta.cameraMake().ifPresent(v -> sb.append(",\"cameraMake\":\"").append(escapeJson(v)).append("\""));
        meta.cameraModel().ifPresent(v -> sb.append(",\"cameraModel\":\"").append(escapeJson(v)).append("\""));
        meta.width().ifPresent(w -> sb.append(",\"width\":").append(w));
        meta.height().ifPresent(h -> sb.append(",\"height\":").append(h));
        if (meta.hasGps()) {
            sb.append(",\"latitude\":").append(meta.gpsLatitude());
            sb.append(",\"longitude\":").append(meta.gpsLongitude());
        }
        sb.append("}");
        System.out.println(sb);
    }

    private String escapeJson(String s) {
        return s.replace("\\", "\\\\").replace("\"", "\\\"");
    }
}
