package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.core.db.DatabaseManager;
import com.filez.core.db.DuplicateRepository;
import com.filez.core.db.FileRepository;
import com.filez.core.interfaces.FileScanner;
import com.filez.core.interfaces.Hasher;
import com.filez.core.registry.Registry;
import com.filez.core.types.DuplicateGroup;
import com.filez.core.types.FileInfo;
import com.filez.core.types.Hashes;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;
import picocli.CommandLine.ParentCommand;

import java.nio.file.Path;
import java.sql.SQLException;
import java.util.*;
import java.util.concurrent.Callable;

@Command(
    name = "duplicates",
    description = "Find duplicate files"
)
public class DuplicatesCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Parameters(index = "0..*", arity = "1..*", paramLabel = "PATH", description = "Directories to scan for duplicates")
    private List<Path> paths;

    @Option(names = {"-e", "--ext"}, split = ",", description = "Filter by extensions")
    private Set<String> extensions;

    @Option(names = {"-L", "--follow-symlinks"}, description = "Follow symbolic links")
    private boolean followSymlinks;

    @Option(names = {"--min-size"}, description = "Minimum file size in bytes", defaultValue = "1")
    private long minSize;

    @Option(names = {"--format"}, description = "Output format (text, json)", defaultValue = "text")
    private String format;

    @Option(names = {"--no-db"}, description = "Don't persist results to database")
    private boolean noDb;

    @Override
    public Integer call() throws SQLException {
        FileScanner scanner = Registry.of(FileScanner.class).getOrThrow(parent.getScannerName());
        Hasher hasher = Registry.of(Hasher.class).getOrThrow(parent.getHasherName());

        DatabaseManager db = null;
        FileRepository fileRepo = null;
        DuplicateRepository dupeRepo = null;

        if (!noDb && !parent.isDryRun()) {
            db = new DatabaseManager(parent.getDatabasePath());
            fileRepo = new FileRepository(db);
            dupeRepo = new DuplicateRepository(db);
        }

        try {
            if (parent.isVerbose()) {
                System.err.println("Scanning for duplicates...");
            }

            List<FileInfo> files = scanner.scan(paths, extensions, followSymlinks);
            
            Map<Long, List<FileInfo>> bySize = new HashMap<>();
            for (FileInfo file : files) {
                if (file.size() >= minSize) {
                    bySize.computeIfAbsent(file.size(), k -> new ArrayList<>()).add(file);
                }
            }

            Map<String, List<FileInfo>> byHash = new HashMap<>();
            int potentialDupes = 0;
            for (List<FileInfo> sameSize : bySize.values()) {
                if (sameSize.size() > 1) {
                    potentialDupes += sameSize.size();
                    for (FileInfo file : sameSize) {
                        String hash = hasher.fast64(file.path());
                        byHash.computeIfAbsent(hash, k -> new ArrayList<>()).add(file);
                        
                        if (fileRepo != null) {
                            FileInfo saved = fileRepo.save(file);
                            fileRepo.saveHashes(saved.id(), Hashes.ofFast(hash));
                        }
                    }
                }
            }

            List<DuplicateGroup> groups = new ArrayList<>();
            for (Map.Entry<String, List<FileInfo>> entry : byHash.entrySet()) {
                if (entry.getValue().size() > 1) {
                    List<Path> filePaths = entry.getValue().stream().map(FileInfo::path).toList();
                    long size = entry.getValue().get(0).size();
                    DuplicateGroup group = new DuplicateGroup(size, entry.getKey(), filePaths);
                    groups.add(group);
                    
                    if (dupeRepo != null) {
                        dupeRepo.saveGroup(group);
                    }
                }
            }

            if (groups.isEmpty()) {
                System.out.println("No duplicates found.");
                return 0;
            }

            long wastedSpace = groups.stream()
                .mapToLong(DuplicateGroup::wastedSpace)
                .sum();

            if ("json".equals(format)) {
                printJson(groups);
            } else {
                printText(groups);
            }

            System.err.printf("%nFound %d duplicate groups (%d files, %.2f MB wasted)%n",
                groups.size(),
                groups.stream().mapToInt(g -> g.files().size()).sum(),
                wastedSpace / (1024.0 * 1024.0));

            if (dupeRepo != null) {
                System.err.println("Results saved to: " + parent.getDatabasePath());
            }

            return 0;
        } finally {
            if (db != null) {
                db.close();
            }
        }
    }

    private void printText(List<DuplicateGroup> groups) {
        int groupNum = 1;
        for (DuplicateGroup group : groups) {
            System.out.printf("=== Group %d (hash: %s, size: %d bytes) ===%n", 
                groupNum++, group.fast64(), group.size());
            for (Path path : group.files()) {
                System.out.println("  " + path);
            }
            System.out.println();
        }
    }

    private void printJson(List<DuplicateGroup> groups) {
        System.out.println("[");
        for (int i = 0; i < groups.size(); i++) {
            DuplicateGroup g = groups.get(i);
            System.out.printf("  {\"hash\":\"%s\",\"size\":%d,\"files\":[%n", 
                g.fast64(), g.size());
            for (int j = 0; j < g.files().size(); j++) {
                System.out.printf("    \"%s\"%s%n", 
                    escapeJson(g.files().get(j).toString()),
                    j < g.files().size() - 1 ? "," : "");
            }
            System.out.printf("  ]}%s%n", i < groups.size() - 1 ? "," : "");
        }
        System.out.println("]");
    }

    private String escapeJson(String s) {
        return s.replace("\\", "\\\\").replace("\"", "\\\"");
    }
}
