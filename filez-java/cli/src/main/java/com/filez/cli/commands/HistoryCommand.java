package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.core.db.DatabaseManager;
import com.filez.core.db.OperationRepository;
import com.filez.core.types.Operation;
import picocli.CommandLine.Command;
import picocli.CommandLine.ParentCommand;

import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.List;
import java.util.concurrent.Callable;

@Command(
    name = "history",
    description = "View operation history"
)
public class HistoryCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Override
    public Integer call() throws Exception {
        try (DatabaseManager db = new DatabaseManager(parent.getDatabasePath())) {
            OperationRepository repo = new OperationRepository(db);
            List<Operation> operations = repo.getAll();

            if (operations.isEmpty()) {
                System.out.println("No operations found in history.");
                return 0;
            }

            System.out.printf("Found %d operations:%n", operations.size());
            DateTimeFormatter fmt = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss").withZone(ZoneId.systemDefault());

            for (Operation op : operations) {
                System.out.printf("[%s] %s (%s)%n", 
                    fmt.format(op.timestamp()), op.type(), op.status());
                System.out.printf("  Source: %s%n", op.sourcePath());
                if (op.destPath() != null) {
                    System.out.printf("  Dest:   %s%n", op.destPath());
                }
                if (op.details() != null) {
                    System.out.printf("  Details: %s%n", op.details());
                }
                System.out.println();
            }
            
            return 0;
        }
    }
}
