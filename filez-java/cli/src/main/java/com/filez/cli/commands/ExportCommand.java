package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.cli.utils.Exporter;
import com.filez.core.interfaces.FileScanner;
import com.filez.core.registry.Registry;
import com.filez.core.types.FileInfo;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;
import picocli.CommandLine.ParentCommand;

import java.nio.file.Path;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Callable;

@Command(
    name = "export",
    description = "Export scan results to a file"
)
public class ExportCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Parameters(index = "0..*", arity = "1..*", paramLabel = "PATH", description = "Directories to scan")
    private List<Path> paths;

    @Option(names = {"-o", "--output"}, required = true, description = "Output file path")
    private Path outputFile;

    @Option(names = {"-f", "--format"}, description = "Format: json, csv, html")
    private String format;

    @Option(names = {"-e", "--ext"}, split = ",", description = "Filter by extensions")
    private Set<String> extensions;

    @Override
    public Integer call() throws Exception {
        FileScanner scanner = Registry.of(FileScanner.class).getOrThrow(parent.getScannerName());
        
        if (parent.isVerbose()) {
            System.err.println("Scanning...");
        }
        
        List<FileInfo> files = scanner.scan(paths, extensions, false);
        
        if (parent.isVerbose()) {
            System.err.printf("Found %d files. Exporting...%n", files.size());
        }

        String fmt = format;
        if (fmt == null) {
            String name = outputFile.getFileName().toString().toLowerCase();
            if (name.endsWith(".json")) fmt = "json";
            else if (name.endsWith(".csv")) fmt = "csv";
            else if (name.endsWith(".html")) fmt = "html";
            else fmt = "json";
        }

        switch (fmt.toLowerCase()) {
            case "json" -> Exporter.exportJson(files, outputFile);
            case "csv" -> Exporter.exportCsv(files, outputFile);
            case "html" -> Exporter.exportHtml(files, outputFile);
            default -> {
                System.err.println("Unknown format: " + fmt);
                return 1;
            }
        }

        System.out.println("Exported " + files.size() + " records to " + outputFile);
        return 0;
    }
}
