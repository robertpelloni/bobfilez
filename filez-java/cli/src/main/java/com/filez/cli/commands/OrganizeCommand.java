package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.core.interfaces.FileScanner;
import com.filez.core.registry.Registry;
import com.filez.core.types.FileInfo;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;
import picocli.CommandLine.ParentCommand;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.time.LocalDate;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicInteger;

@Command(
    name = "organize",
    description = "Organize files into folders based on rules"
)
public class OrganizeCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Parameters(index = "0", paramLabel = "SOURCE", description = "Source directory")
    private Path sourceDir;

    @Parameters(index = "1", paramLabel = "DEST", description = "Destination directory")
    private Path destDir;

    @Option(names = {"-e", "--ext"}, split = ",", description = "Filter by extensions")
    private Set<String> extensions;

    @Option(names = {"--by"}, description = "Organization strategy: date, extension, size", defaultValue = "date")
    private String strategy;

    @Option(names = {"--date-format"}, description = "Date folder format", defaultValue = "yyyy/MM")
    private String dateFormat;

    @Option(names = {"--copy"}, description = "Copy instead of move")
    private boolean copyMode;

    @Option(names = {"--format"}, description = "Output format (text, json)", defaultValue = "text")
    private String format;

    @Override
    public Integer call() {
        if (!Files.isDirectory(sourceDir)) {
            System.err.println("Source is not a directory: " + sourceDir);
            return 1;
        }

        FileScanner scanner = Registry.of(FileScanner.class).getOrThrow(parent.getScannerName());
        List<FileInfo> files = scanner.scan(List.of(sourceDir), extensions, false);

        if (parent.isVerbose()) {
            System.err.printf("Organizing %d files by %s%n", files.size(), strategy);
        }

        AtomicInteger moved = new AtomicInteger();
        AtomicInteger errors = new AtomicInteger();
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern(dateFormat);

        for (FileInfo file : files) {
            try {
                Path targetFolder = computeTargetFolder(file, formatter);
                Path targetPath = destDir.resolve(targetFolder).resolve(file.path().getFileName());

                if (parent.isDryRun()) {
                    printAction(file.path(), targetPath);
                } else {
                    Files.createDirectories(targetPath.getParent());
                    if (copyMode) {
                        Files.copy(file.path(), targetPath, StandardCopyOption.REPLACE_EXISTING);
                    } else {
                        Files.move(file.path(), targetPath, StandardCopyOption.REPLACE_EXISTING);
                    }
                    printAction(file.path(), targetPath);
                }
                moved.incrementAndGet();
            } catch (IOException e) {
                errors.incrementAndGet();
                if (parent.isVerbose()) {
                    System.err.println("Error processing " + file.path() + ": " + e.getMessage());
                }
            }
        }

        String action = parent.isDryRun() ? "Would process" : (copyMode ? "Copied" : "Moved");
        System.err.printf("%n%s %d files (%d errors)%n", action, moved.get(), errors.get());
        
        return errors.get() > 0 && moved.get() == 0 ? 1 : 0;
    }

    private Path computeTargetFolder(FileInfo file, DateTimeFormatter formatter) {
        return switch (strategy.toLowerCase()) {
            case "date" -> {
                LocalDate date = file.modifiedTime().atZone(ZoneId.systemDefault()).toLocalDate();
                yield Path.of(date.format(formatter));
            }
            case "extension" -> {
                String ext = file.extension();
                yield Path.of(ext.isEmpty() ? "no-extension" : ext);
            }
            case "size" -> {
                long mb = file.size() / (1024 * 1024);
                if (mb < 1) yield Path.of("small");
                else if (mb < 10) yield Path.of("medium");
                else if (mb < 100) yield Path.of("large");
                else yield Path.of("huge");
            }
            default -> Path.of("other");
        };
    }

    private void printAction(Path source, Path target) {
        if ("json".equals(format)) {
            System.out.printf("{\"source\":\"%s\",\"target\":\"%s\"}%n",
                escapeJson(source.toString()), escapeJson(target.toString()));
        } else {
            String action = copyMode ? "COPY" : "MOVE";
            System.out.printf("%s: %s -> %s%n", action, source, target);
        }
    }

    private String escapeJson(String s) {
        return s.replace("\\", "\\\\").replace("\"", "\\\"");
    }
}
