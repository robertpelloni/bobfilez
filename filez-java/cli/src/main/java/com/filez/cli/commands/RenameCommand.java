package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.core.db.DatabaseManager;
import com.filez.core.db.OperationRepository;
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
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicInteger;

@Command(
    name = "rename",
    description = "Batch rename files using a pattern"
)
public class RenameCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Parameters(index = "0", description = "Source directories", arity = "1..*")
    private List<Path> sourceDirs;

    @Option(names = {"-p", "--pattern"}, required = true, description = "Rename pattern (vars: {name}, {ext}, {parent}, {date}, {counter})")
    private String pattern;

    @Option(names = {"-e", "--ext"}, split = ",", description = "Filter by extensions")
    private Set<String> extensions;

    @Option(names = {"--confirm"}, description = "Execute without dry-run (deprecated, use global --dry-run=false)")
    private boolean confirm;

    @Override
    public Integer call() throws Exception {
        FileScanner scanner = Registry.of(FileScanner.class).getOrThrow(parent.getScannerName());
        List<FileInfo> files = scanner.scan(sourceDirs, extensions, false);

        if (parent.isVerbose()) {
            System.err.printf("Found %d files to rename%n", files.size());
        }

        AtomicInteger success = new AtomicInteger();
        AtomicInteger errors = new AtomicInteger();
        AtomicInteger counter = new AtomicInteger(1);
        DateTimeFormatter dateFmt = DateTimeFormatter.ofPattern("yyyy-MM-dd");

        try (DatabaseManager db = new DatabaseManager(parent.getDatabasePath())) {
            OperationRepository repo = new OperationRepository(db);

            for (FileInfo file : files) {
                String newName = applyPattern(file, counter.getAndIncrement(), dateFmt);
                Path target = file.path().resolveSibling(newName);

                if (target.equals(file.path())) {
                    continue;
                }

                if (parent.isDryRun()) {
                    System.out.printf("[DRY RUN] Rename: %s -> %s%n", file.path(), target);
                } else {
                    try {
                        if (Files.exists(target)) {
                            throw new IOException("Target exists: " + target);
                        }
                        Files.move(file.path(), target);
                        repo.log("RENAME", file.path().toString(), target.toString(), "SUCCESS", null);
                        System.out.printf("Renamed: %s -> %s%n", file.fileName(), target.getFileName());
                        success.incrementAndGet();
                    } catch (Exception e) {
                        repo.log("RENAME", file.path().toString(), target.toString(), "FAILED", e.getMessage());
                        System.err.printf("Error renaming %s: %s%n", file.path(), e.getMessage());
                        errors.incrementAndGet();
                    }
                }
            }
        }

        System.err.printf("Renamed %d files (%d errors)%n", success.get(), errors.get());
        return errors.get() > 0 ? 1 : 0;
    }

    private String applyPattern(FileInfo file, int count, DateTimeFormatter dateFmt) {
        String name = file.fileName();
        String ext = file.extension();
        String baseName = name.substring(0, name.length() - (ext.isEmpty() ? 0 : ext.length() + 1));
        String parentName = file.path().getParent() != null ? file.path().getParent().getFileName().toString() : "";
        String date = file.modifiedTime().atZone(ZoneId.systemDefault()).format(dateFmt);

        return pattern
            .replace("{name}", baseName)
            .replace("{ext}", ext)
            .replace("{parent}", parentName)
            .replace("{date}", date)
            .replace("{counter}", String.format("%04d", count));
    }
}
