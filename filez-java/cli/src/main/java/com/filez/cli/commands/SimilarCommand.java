package com.filez.cli.commands;

import com.filez.cli.FilezApp;
import com.filez.core.interfaces.FileScanner;
import com.filez.core.interfaces.PerceptualHasher;
import com.filez.core.registry.Registry;
import com.filez.core.types.FileInfo;
import com.filez.core.types.PerceptualHash;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;
import picocli.CommandLine.ParentCommand;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.*;
import java.util.concurrent.Callable;

@Command(
    name = "similar",
    description = "Find visually similar images"
)
public class SimilarCommand implements Callable<Integer> {

    @ParentCommand
    private FilezApp parent;

    @Parameters(index = "0", paramLabel = "REFERENCE", description = "Reference image")
    private Path referenceImage;

    @Parameters(index = "1..*", arity = "1..*", paramLabel = "PATH", description = "Directories to search")
    private List<Path> paths;

    @Option(names = {"-e", "--ext"}, split = ",", description = "Filter by extensions", 
            defaultValue = "jpg,jpeg,png,bmp,gif")
    private Set<String> extensions;

    @Option(names = {"--algo"}, description = "Algorithm: dhash, ahash", defaultValue = "dhash")
    private String algorithm;

    @Option(names = {"-t", "--threshold"}, description = "Distance threshold (0-64)", defaultValue = "10")
    private int threshold;

    @Override
    public Integer call() throws Exception {
        if (!Files.exists(referenceImage)) {
            System.err.println("Reference image not found: " + referenceImage);
            return 1;
        }

        FileScanner scanner = Registry.of(FileScanner.class).getOrThrow(parent.getScannerName());
        PerceptualHasher hasher = Registry.of(PerceptualHasher.class).get(algorithm)
            .orElseThrow(() -> new IllegalArgumentException("Unknown algorithm: " + algorithm));

        if (parent.isVerbose()) {
            System.err.println("Computing hash for reference image...");
        }

        Optional<PerceptualHash> refHashOpt = hasher.compute(referenceImage);
        if (refHashOpt.isEmpty()) {
            System.err.println("Failed to compute hash for reference image (unsupported format?)");
            return 1;
        }
        PerceptualHash refHash = refHashOpt.get();
        
        if (parent.isVerbose()) {
            System.err.printf("Reference hash (%s): %s%n", algorithm, Long.toHexString(refHash.value()));
            System.err.println("Scanning directories...");
        }

        List<FileInfo> files = scanner.scan(paths, extensions, false);
        List<Match> matches = new ArrayList<>();

        for (FileInfo file : files) {
            // Skip the reference image itself if it's in the scan path
            if (file.path().toAbsolutePath().equals(referenceImage.toAbsolutePath())) continue;

            Optional<PerceptualHash> fileHash = hasher.compute(file.path());
            if (fileHash.isPresent()) {
                int distance = refHash.hammingDistance(fileHash.get());
                if (distance <= threshold) {
                    matches.add(new Match(file.path(), distance));
                }
            }
        }

        matches.sort(Comparator.comparingInt(Match::distance));

        System.out.printf("Found %d similar images (threshold %d):%n", matches.size(), threshold);
        for (Match match : matches) {
            System.out.printf("[%d] %s%n", match.distance(), match.path());
        }

        return 0;
    }

    private record Match(Path path, int distance) {}
}
