#include <gtest/gtest.h>
#include "fo/core/export.hpp"
#include <sstream>

using namespace fo::core;

TEST(ExporterTest, JsonEscape) {
    EXPECT_EQ(Exporter::json_escape("hello"), "hello");
    EXPECT_EQ(Exporter::json_escape("hello\"world"), "hello\\\"world");
    EXPECT_EQ(Exporter::json_escape("path\\to\\file"), "path\\\\to\\\\file");
    EXPECT_EQ(Exporter::json_escape("line1\nline2"), "line1\\nline2");
    EXPECT_EQ(Exporter::json_escape("tab\there"), "tab\\there");
}

TEST(ExporterTest, CsvEscape) {
    EXPECT_EQ(Exporter::csv_escape("hello"), "hello");
    EXPECT_EQ(Exporter::csv_escape("hello,world"), "\"hello,world\"");
    EXPECT_EQ(Exporter::csv_escape("hello\"world"), "\"hello\"\"world\"");
    EXPECT_EQ(Exporter::csv_escape("line1\nline2"), "\"line1\nline2\"");
}

TEST(ExporterTest, HtmlEscape) {
    EXPECT_EQ(Exporter::html_escape("hello"), "hello");
    EXPECT_EQ(Exporter::html_escape("<script>"), "&lt;script&gt;");
    EXPECT_EQ(Exporter::html_escape("a & b"), "a &amp; b");
    EXPECT_EQ(Exporter::html_escape("\"quoted\""), "&quot;quoted&quot;");
}

TEST(ExporterTest, FormatSize) {
    EXPECT_EQ(Exporter::format_size(0), "0.00 B");
    EXPECT_EQ(Exporter::format_size(512), "512.00 B");
    EXPECT_EQ(Exporter::format_size(1024), "1.00 KB");
    EXPECT_EQ(Exporter::format_size(1536), "1.50 KB");
    EXPECT_EQ(Exporter::format_size(1048576), "1.00 MB");
    EXPECT_EQ(Exporter::format_size(1073741824), "1.00 GB");
}

TEST(ExporterTest, ComputeStats) {
    std::vector<FileInfo> files;
    
    FileInfo f1;
    f1.uri = "file1.txt";
    f1.size = 1000;
    f1.is_dir = false;
    files.push_back(f1);
    
    FileInfo f2;
    f2.uri = "file2.txt";
    f2.size = 2000;
    f2.is_dir = false;
    files.push_back(f2);
    
    FileInfo d1;
    d1.uri = "dir1";
    d1.size = 0;
    d1.is_dir = true;
    files.push_back(d1);
    
    std::vector<DuplicateGroup> duplicates;
    DuplicateGroup g1;
    g1.size = 1000;
    g1.fast64 = "abc123";
    g1.files.push_back(f1);
    g1.files.push_back(f1);
    duplicates.push_back(g1);
    
    auto stats = Exporter::compute_stats(files, duplicates);
    
    EXPECT_EQ(stats.total_files, 2);
    EXPECT_EQ(stats.total_directories, 1);
    EXPECT_EQ(stats.total_size, 3000);
    EXPECT_EQ(stats.duplicate_groups, 1);
    EXPECT_EQ(stats.duplicate_files, 2);
    EXPECT_EQ(stats.duplicate_size, 1000); // Wasted space = size * (count - 1)
}

TEST(ExporterTest, ToCsv) {
    std::vector<FileInfo> files;
    
    FileInfo f1;
    f1.id = 1;
    f1.uri = "test.txt";
    f1.size = 100;
    f1.is_dir = false;
    f1.mtime = std::chrono::file_clock::now();
    files.push_back(f1);
    
    std::ostringstream oss;
    Exporter::to_csv(oss, files);
    
    std::string output = oss.str();
    EXPECT_TRUE(output.find("id,path,size,size_human,mtime,is_dir") != std::string::npos);
    EXPECT_TRUE(output.find("test.txt") != std::string::npos);
    EXPECT_TRUE(output.find("100") != std::string::npos);
}

TEST(ExporterTest, ToJson) {
    std::vector<FileInfo> files;
    
    FileInfo f1;
    f1.id = 1;
    f1.uri = "test.txt";
    f1.size = 100;
    f1.is_dir = false;
    f1.mtime = std::chrono::file_clock::now();
    files.push_back(f1);
    
    std::vector<DuplicateGroup> duplicates;
    ScanStats stats = Exporter::compute_stats(files, duplicates);
    
    std::ostringstream oss;
    Exporter::to_json(oss, files, duplicates, stats);
    
    std::string output = oss.str();
    EXPECT_TRUE(output.find("\"stats\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"files\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"duplicates\"") != std::string::npos);
    EXPECT_TRUE(output.find("test.txt") != std::string::npos);
}

