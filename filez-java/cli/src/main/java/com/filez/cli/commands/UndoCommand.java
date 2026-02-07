package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.core.db.DatabaseManager;
import com.filez.core.db.OperationRepository;
import com.filez.core.types.Operation;
import picocli.CommandLine.Command;
import picocli.CommandLine.ParentCommand;

import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.Optional;
import java.util.concurrent.Callable;

@Command(
    name = "undo",
    description = "Undo the last operation"
)
public class UndoCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Override
    public Integer call() throws Exception {
        try (DatabaseManager db = new DatabaseManager(parent.getDatabasePath())) {
            OperationRepository repo = new OperationRepository(db);
            
            Optional<Operation> lastOp = repo.getLast();
            if (lastOp.isEmpty()) {
                System.out.println("Nothing to undo.");
                return 0;
            }

            Operation op = lastOp.get();
            System.out.printf("Undoing %s: %s -> %s... ", op.type(), op.sourcePath(), op.destPath());

            try {
                switch (op.type()) {
                    case "RENAME":
                    case "MOVE":
                        undoMove(op.destPath(), op.sourcePath());
                        break;
                    case "COPY":
                        undoCopy(op.destPath());
                        break;
                    case "DELETE":
                        System.out.println("FAILED (Cannot undo deletion)");
                        return 1;
                    default:
                        System.out.println("FAILED (Unknown operation type)");
                        return 1;
                }
                
                repo.delete(op.id());
                System.out.println("SUCCESS");
                return 0;

            } catch (Exception e) {
                System.out.println("FAILED: " + e.getMessage());
                return 1;
            }
        }
    }

    private void undoMove(String currentPath, String originalPath) throws Exception {
        Path src = Path.of(currentPath);
        Path dest = Path.of(originalPath);
        
        if (!Files.exists(src)) {
            throw new Exception("File not found: " + src);
        }
        if (Files.exists(dest)) {
            throw new Exception("Destination already exists: " + dest);
        }
        
        Files.move(src, dest, StandardCopyOption.ATOMIC_MOVE);
    }

    private void undoCopy(String copiedPath) throws Exception {
        Path copy = Path.of(copiedPath);
        if (Files.exists(copy)) {
            Files.delete(copy);
        }
    }
}
