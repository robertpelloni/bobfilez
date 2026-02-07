package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.core.interfaces.FileScanner;
import com.filez.core.registry.Registry;
import com.filez.core.types.FileInfo;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;
import picocli.CommandLine.ParentCommand;

import java.nio.file.Path;
import java.util.*;
import java.util.concurrent.Callable;

@Command(
    name = "stats",
    description = "Show statistics about files"
)
public class StatsCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Parameters(index = "0..*", arity = "1..*", paramLabel = "PATH", description = "Directories to scan")
    private List<Path> paths;

    @Option(names = {"-e", "--ext"}, split = ",", description = "Filter by extensions")
    private Set<String> extensions;

    @Override
    public Integer call() {
        FileScanner scanner = Registry.of(FileScanner.class).getOrThrow(parent.getScannerName());
        List<FileInfo> files = scanner.scan(paths, extensions, false);

        long totalSize = 0;
        Map<String, Integer> extCounts = new HashMap<>();
        Map<String, Long> extSizes = new HashMap<>();

        for (FileInfo f : files) {
            totalSize += f.size();
            String ext = f.extension();
            if (ext.isEmpty()) ext = "(no extension)";
            
            extCounts.put(ext, extCounts.getOrDefault(ext, 0) + 1);
            extSizes.put(ext, extSizes.getOrDefault(ext, 0L) + f.size());
        }

        System.out.println("=== Statistics ===");
        System.out.printf("Total Files: %d%n", files.size());
        System.out.printf("Total Size:  %s%n", formatSize(totalSize));
        System.out.println();
        System.out.println("By Extension:");
        
        extCounts.entrySet().stream()
            .sorted(Map.Entry.<String, Integer>comparingByValue().reversed())
            .limit(20)
            .forEach(e -> {
                String ext = e.getKey();
                int count = e.getValue();
                long size = extSizes.get(ext);
                System.out.printf("  %-15s %6d files  %10s%n", ext, count, formatSize(size));
            });

        return 0;
    }

    private String formatSize(long bytes) {
        if (bytes < 1024) return bytes + " B";
        int exp = (int) (Math.log(bytes) / Math.log(1024));
        String pre = "KMGTPE".charAt(exp - 1) + "";
        return String.format("%.1f %sB", bytes / Math.pow(1024, exp), pre);
    }
}
