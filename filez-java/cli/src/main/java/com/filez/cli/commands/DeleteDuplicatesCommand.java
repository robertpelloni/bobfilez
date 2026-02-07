package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.core.db.DatabaseManager;
import com.filez.core.db.DuplicateRepository;
import com.filez.core.db.OperationRepository;
import com.filez.core.types.DuplicateGroup;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.ParentCommand;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.stream.Collectors;

@Command(
    name = "delete-duplicates",
    description = "Delete duplicate files, keeping one based on strategy"
)
public class DeleteDuplicatesCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Option(names = {"--keep"}, required = true, description = "Strategy to keep one file: oldest, newest, shortest, longest")
    private String keepStrategy;

    @Option(names = {"--confirm"}, description = "Execute without dry-run")
    private boolean confirm;

    @Override
    public Integer call() throws Exception {
        if (!confirm) {
            System.err.println("DRY RUN: Use --confirm to actually delete files.");
        }

        try (DatabaseManager db = new DatabaseManager(parent.getDatabasePath())) {
            DuplicateRepository dupeRepo = new DuplicateRepository(db);
            OperationRepository opRepo = new OperationRepository(db);

            List<DuplicateGroup> groups = dupeRepo.findAll();
            if (groups.isEmpty()) {
                System.out.println("No duplicate groups found in database.");
                return 0;
            }

            System.out.printf("Processing %d duplicate groups...%n", groups.size());

            AtomicInteger deletedCount = new AtomicInteger();
            AtomicInteger keptCount = new AtomicInteger();
            AtomicInteger errors = new AtomicInteger();

            for (DuplicateGroup group : groups) {
                if (group.files().size() < 2) continue;

                List<Path> sorted = sortFiles(group.files(), keepStrategy);
                Path toKeep = sorted.get(0);
                List<Path> toDelete = sorted.subList(1, sorted.size());

                keptCount.incrementAndGet();

                for (Path p : toDelete) {
                    if (confirm) {
                        try {
                            Files.delete(p);
                            opRepo.log("DELETE", p.toString(), null, "SUCCESS", "Duplicate of " + toKeep);
                            System.out.println("Deleted: " + p);
                            deletedCount.incrementAndGet();
                        } catch (IOException e) {
                            opRepo.log("DELETE", p.toString(), null, "FAILED", e.getMessage());
                            System.err.println("Error deleting " + p + ": " + e.getMessage());
                            errors.incrementAndGet();
                        }
                    } else {
                        System.out.println("[DRY RUN] Would delete: " + p + " (keeping " + toKeep + ")");
                        deletedCount.incrementAndGet();
                    }
                }
                
                if (confirm && errors.get() == 0) {
                     
                }
            }

            System.out.printf("%nResult: Kept %d files, %s %d files (%d errors)%n",
                keptCount.get(), confirm ? "Deleted" : "Would delete", deletedCount.get(), errors.get());

            return errors.get() > 0 ? 1 : 0;
        }
    }

    private List<Path> sortFiles(List<Path> files, String strategy) {
        Comparator<Path> comparator = switch (strategy.toLowerCase()) {
            case "shortest" -> Comparator.comparingInt((Path p) -> p.toString().length());
            case "longest" -> Comparator.comparingInt((Path p) -> p.toString().length()).reversed();
            case "oldest" -> Comparator.comparing(this::getCreationTime);
            case "newest" -> Comparator.comparing(this::getCreationTime).reversed();
            default -> throw new IllegalArgumentException("Unknown strategy: " + strategy);
        };

        return files.stream().sorted(comparator).collect(Collectors.toList());
    }

    private java.time.Instant getCreationTime(Path p) {
        try {
            return Files.readAttributes(p, BasicFileAttributes.class).creationTime().toInstant();
        } catch (IOException e) {
            return java.time.Instant.MAX;
        }
    }
}
