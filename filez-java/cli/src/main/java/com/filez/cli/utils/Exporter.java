package com.filez.cli.utils;

import com.filez.core.types.FileInfo;

import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.List;

public class Exporter {

    public static void exportJson(List<FileInfo> files, Path output) throws IOException {
        try (PrintWriter writer = new PrintWriter(Files.newBufferedWriter(output))) {
            writer.println("[");
            for (int i = 0; i < files.size(); i++) {
                FileInfo f = files.get(i);
                writer.printf("  {\"path\":\"%s\",\"size\":%d,\"mtime\":\"%s\",\"isDir\":%b}%s%n",
                    escapeJson(f.path().toString()),
                    f.size(),
                    f.modifiedTime(),
                    f.isDirectory(),
                    i < files.size() - 1 ? "," : ""
                );
            }
            writer.println("]");
        }
    }

    public static void exportCsv(List<FileInfo> files, Path output) throws IOException {
        try (PrintWriter writer = new PrintWriter(Files.newBufferedWriter(output))) {
            writer.println("Path,Size,ModifiedTime,IsDirectory");
            DateTimeFormatter fmt = DateTimeFormatter.ISO_LOCAL_DATE_TIME.withZone(ZoneId.systemDefault());
            for (FileInfo f : files) {
                writer.printf("\"%s\",%d,%s,%b%n",
                    escapeCsv(f.path().toString()),
                    f.size(),
                    fmt.format(f.modifiedTime()),
                    f.isDirectory()
                );
            }
        }
    }

    public static void exportHtml(List<FileInfo> files, Path output) throws IOException {
        try (PrintWriter writer = new PrintWriter(Files.newBufferedWriter(output))) {
            writer.println("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Scan Report</title>");
            writer.println("<style>table { border-collapse: collapse; width: 100%; } th, td { border: 1px solid #ddd; padding: 8px; } tr:nth-child(even) { background-color: #f2f2f2; } th { background-color: #4CAF50; color: white; }</style>");
            writer.println("</head><body>");
            writer.println("<h1>Scan Report</h1>");
            writer.printf("<p>Total files: %d</p>", files.size());
            writer.println("<table><thead><tr><th>Path</th><th>Size</th><th>Modified</th><th>Type</th></tr></thead><tbody>");
            
            DateTimeFormatter fmt = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss").withZone(ZoneId.systemDefault());
            for (FileInfo f : files) {
                writer.printf("<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>%n",
                    escapeHtml(f.path().toString()),
                    formatSize(f.size()),
                    fmt.format(f.modifiedTime()),
                    f.isDirectory() ? "DIR" : "FILE"
                );
            }
            writer.println("</tbody></table></body></html>");
        }
    }

    private static String escapeJson(String s) {
        return s.replace("\\", "\\\\").replace("\"", "\\\"");
    }

    private static String escapeCsv(String s) {
        return s.replace("\"", "\"\"");
    }

    private static String escapeHtml(String s) {
        return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");
    }

    private static String formatSize(long bytes) {
        if (bytes < 1024) return bytes + " B";
        int exp = (int) (Math.log(bytes) / Math.log(1024));
        String pre = "KMGTPE".charAt(exp - 1) + "";
        return String.format("%.1f %sB", bytes / Math.pow(1024, exp), pre);
    }
}
