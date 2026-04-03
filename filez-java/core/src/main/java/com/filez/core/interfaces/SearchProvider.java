package com.filez.core.interfaces;

import java.nio.file.Path;
import java.util.List;
import java.util.Optional;

/**
 * Comprehensive search interface (filename + content).
 */
public interface SearchProvider {
    
    record ContentMatch(
        long line,
        long column,
        String text,
        String match
    ) {}

    record SearchResult(
        Path path,
        long size,
        long modified,
        List<ContentMatch> matches
    ) {}

    enum MatchMode { Literal, Wildcard, Regex, Fuzzy }

    record SearchOptions(
        String query,
        MatchMode mode,
        boolean caseSensitive,
        boolean recursive,
        boolean searchContent,
        String contentQuery
    ) {}

    List<SearchResult> search(List<Path> roots, SearchOptions opts);
}
