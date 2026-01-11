package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.core.interfaces.FileScanner;
import com.filez.core.interfaces.Hasher;
import com.filez.core.registry.Registry;
import com.filez.core.types.FileInfo;
import com.filez.core.types.Hashes;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;
import picocli.CommandLine.ParentCommand;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Callable;

@Command(
    name = "hash",
    description = "Compute file hashes"
)
public class HashCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Parameters(index = "0..*", arity = "1..*", paramLabel = "PATH", description = "Files or directories to hash")
    private List<Path> paths;

    @Option(names = {"-e", "--ext"}, split = ",", description = "Filter by extensions when scanning directories")
    private Set<String> extensions;

    @Option(names = {"--fast-only"}, description = "Only compute fast hash (skip strong hash)")
    private boolean fastOnly;

    @Option(names = {"--format"}, description = "Output format (text, json)", defaultValue = "text")
    private String format;

    @Override
    public Integer call() {
        Hasher hasher = Registry.of(Hasher.class).getOrThrow(parent.getHasherName());
        FileScanner scanner = Registry.of(FileScanner.class).getOrThrow(parent.getScannerName());

        for (Path path : paths) {
            if (Files.isDirectory(path)) {
                List<FileInfo> files = scanner.scan(List.of(path), extensions, false);
                for (FileInfo file : files) {
                    hashAndPrint(hasher, file.path());
                }
            } else if (Files.isRegularFile(path)) {
                hashAndPrint(hasher, path);
            } else {
                System.err.println("Skipping: " + path + " (not a file or directory)");
            }
        }

        return 0;
    }

    private void hashAndPrint(Hasher hasher, Path file) {
        try {
            Hashes hashes = fastOnly ? hasher.hashFastOnly(file) : hasher.hash(file);
            if ("json".equals(format)) {
                printJson(file, hashes);
            } else {
                printText(file, hashes);
            }
        } catch (Exception e) {
            System.err.println("Error hashing " + file + ": " + e.getMessage());
        }
    }

    private void printText(Path file, Hashes hashes) {
        String strong = hashes.strong().orElse("-");
        System.out.printf("%s  %s  %s%n", hashes.fast64(), strong, file);
    }

    private void printJson(Path file, Hashes hashes) {
        System.out.printf("{\"path\":\"%s\",\"fast64\":\"%s\",\"strong\":%s}%n",
            escapeJson(file.toString()),
            hashes.fast64(),
            hashes.strong().map(s -> "\"" + s + "\"").orElse("null")
        );
    }

    private String escapeJson(String s) {
        return s.replace("\\", "\\\\").replace("\"", "\\\"");
    }
}
