package com.filez.cli;

import com.filez.cli.commands.*;
import com.filez.core.impl.CoreModule;
import picocli.CommandLine;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;

import java.nio.file.Path;
import java.util.concurrent.Callable;

@Command(
    name = "filez",
    mixinStandardHelpOptions = true,
    version = "filez 2.1.0",
    description = "File organization and deduplication tool",
    subcommands = {
        ScanCommand.class,
        HashCommand.class,
        DuplicatesCommand.class,
        MetadataCommand.class,
        OrganizeCommand.class,
        RenameCommand.class,
        DeleteDuplicatesCommand.class,
        UndoCommand.class,
        CommandLine.HelpCommand.class
    }
)
public class FilezApp implements Callable<Integer> {

    @Option(names = {"-d", "--database"}, description = "Database file path", defaultValue = "filez.db")
    Path databasePath;

    @Option(names = {"--scanner"}, description = "Scanner implementation (nio, win32)", defaultValue = "nio")
    String scannerName;

    @Option(names = {"--hasher"}, description = "Hasher implementation (java, blake3)", defaultValue = "java")
    String hasherName;

    @Option(names = {"-v", "--verbose"}, description = "Enable verbose output")
    boolean verbose;

    @Option(names = {"--dry-run"}, description = "Show what would be done without making changes")
    boolean dryRun;

    public static void main(String[] args) {
        CoreModule.register();
        int exitCode = new CommandLine(new FilezApp())
            .setExecutionExceptionHandler(new ExceptionHandler())
            .execute(args);
        System.exit(exitCode);
    }

    @Override
    public Integer call() {
        CommandLine.usage(this, System.out);
        return 0;
    }

    public Path getDatabasePath() { return databasePath; }
    public String getScannerName() { return scannerName; }
    public String getHasherName() { return hasherName; }
    public boolean isVerbose() { return verbose; }
    public boolean isDryRun() { return dryRun; }

    private static class ExceptionHandler implements CommandLine.IExecutionExceptionHandler {
        @Override
        public int handleExecutionException(Exception ex, CommandLine cmd, CommandLine.ParseResult parseResult) {
            cmd.getErr().println(cmd.getColorScheme().errorText(ex.getMessage()));
            if (parseResult.hasMatchedOption("verbose")) {
                ex.printStackTrace(cmd.getErr());
            }
            return 1;
        }
    }
}
