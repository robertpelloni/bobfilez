package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.core.db.DatabaseManager;
import com.filez.core.db.FileRepository;
import com.filez.core.interfaces.FileScanner;
import com.filez.core.registry.Registry;
import com.filez.core.types.FileInfo;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;
import picocli.CommandLine.ParentCommand;

import java.nio.file.Path;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicLong;

@Command(
    name = "scan",
    description = "Scan directories for files"
)
public class ScanCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Parameters(index = "0..*", arity = "1..*", paramLabel = "PATH", description = "Directories to scan")
    private List<Path> paths;

    @Option(names = {"-e", "--ext"}, split = ",", description = "Filter by extensions (e.g., jpg,png,mp3)")
    private Set<String> extensions;

    @Option(names = {"-L", "--follow-symlinks"}, description = "Follow symbolic links")
    private boolean followSymlinks;

    @Option(names = {"--format"}, description = "Output format (text, json)", defaultValue = "text")
    private String format;

    @Option(names = {"--incremental"}, description = "Only scan new/modified files")
    private boolean incremental;

    @Option(names = {"--no-db"}, description = "Don't persist results to database")
    private boolean noDb;

    private static final int BATCH_SIZE = 1000;

    @Override
    public Integer call() throws SQLException {
        FileScanner scanner = Registry.of(FileScanner.class).getOrThrow(parent.getScannerName());
        
        AtomicLong fileCount = new AtomicLong();
        AtomicLong totalSize = new AtomicLong();
        List<FileInfo> batch = new ArrayList<>(BATCH_SIZE);
        
        DatabaseManager db = null;
        FileRepository repo = null;
        
        if (!noDb && !parent.isDryRun()) {
            db = new DatabaseManager(parent.getDatabasePath());
            repo = new FileRepository(db);
        }
        
        if (parent.isVerbose()) {
            System.err.println("Scanning " + paths.size() + " path(s) with scanner: " + parent.getScannerName());
            if (db != null) {
                System.err.println("Persisting to database: " + parent.getDatabasePath());
            }
        }

        final FileRepository finalRepo = repo;
        final DatabaseManager finalDb = db;

        try {
            scanner.scanAsync(paths, extensions, followSymlinks,
                file -> {
                    fileCount.incrementAndGet();
                    totalSize.addAndGet(file.size());
                    
                    if ("json".equals(format)) {
                        printJson(file);
                    } else {
                        printText(file);
                    }
                    
                    if (finalRepo != null) {
                        batch.add(file);
                        if (batch.size() >= BATCH_SIZE) {
                            flushBatch(finalRepo, batch);
                        }
                    }
                },
                dir -> {
                    if (parent.isVerbose()) {
                        System.err.println("Scanning: " + dir);
                    }
                }
            );
            
            if (finalRepo != null && !batch.isEmpty()) {
                flushBatch(finalRepo, batch);
            }
        } finally {
            if (finalDb != null) {
                finalDb.close();
            }
        }

        System.err.printf("%nScanned %d files (%.2f MB)%n", 
            fileCount.get(), totalSize.get() / (1024.0 * 1024.0));
        
        if (repo != null) {
            System.err.println("Results saved to: " + parent.getDatabasePath());
        }
        
        return 0;
    }

    private void flushBatch(FileRepository repo, List<FileInfo> batch) {
        try {
            repo.saveBatch(batch);
            batch.clear();
        } catch (SQLException e) {
            throw new RuntimeException("Failed to persist files to database", e);
        }
    }

    private void printText(FileInfo file) {
        System.out.printf("%12d  %s%n", file.size(), file.path());
    }

    private void printJson(FileInfo file) {
        System.out.printf("{\"path\":\"%s\",\"size\":%d,\"mtime\":\"%s\"}%n",
            escapeJson(file.path().toString()),
            file.size(),
            file.modifiedTime()
        );
    }

    private String escapeJson(String s) {
        return s.replace("\\", "\\\\").replace("\"", "\\\"");
    }
}
