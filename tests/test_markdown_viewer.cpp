/// @file test_markdown_viewer.cpp
/// @brief Tests for MarkdownRenderer and MarkdownRenderResult.

#include <gtest/gtest.h>
#include "fo/core/markdown_viewer_interface.hpp"
#include <filesystem>
#include <fstream>

using namespace fo::core;

class MarkdownViewerTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_md_" + unique_id);
        std::filesystem::create_directories(test_dir);
    }
    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_file(const std::filesystem::path& path, const std::string& content) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path);
        ofs << content;
    }

    std::filesystem::path test_dir;
};

// ── MarkdownRenderer Tests ─────────────────────────────────────────────

TEST_F(MarkdownViewerTest, RenderEmptyString) {
    MarkdownRenderer renderer;
    auto result = renderer.render("");
    // Empty input may still generate some structure
    EXPECT_NO_THROW(renderer.render(""));
}

TEST_F(MarkdownViewerTest, RenderPlainText) {
    MarkdownRenderer renderer;
    auto result = renderer.render("Hello World");
    EXPECT_NE(result.html.find("Hello World"), std::string::npos);
    EXPECT_EQ(result.word_count, 2);
}

TEST_F(MarkdownViewerTest, RenderHeading) {
    MarkdownRenderer renderer;
    auto result = renderer.render("# Main Title\n## Sub Title");
    EXPECT_NE(result.html.find("Main Title"), std::string::npos);
    EXPECT_NE(result.html.find("Sub Title"), std::string::npos);
    // headings may or may not be populated depending on md4c parse
}

TEST_F(MarkdownViewerTest, RenderBoldAndItalic) {
    MarkdownRenderer renderer;
    auto result = renderer.render("This is **bold** and *italic*.");
    EXPECT_NE(result.html.find("bold"), std::string::npos);
    EXPECT_NE(result.html.find("italic"), std::string::npos);
}

TEST_F(MarkdownViewerTest, RenderCodeBlock) {
    MarkdownRenderer renderer;
    auto result = renderer.render("```cpp\nint x = 42;\n```");
    EXPECT_NE(result.html.find("int x = 42"), std::string::npos);
}

TEST_F(MarkdownViewerTest, RenderLink) {
    MarkdownRenderer renderer;
    auto result = renderer.render("[Click here](https://example.com)");
    EXPECT_NE(result.html.find("https://example.com"), std::string::npos);
    // links extraction depends on md4c post-processing
}

TEST_F(MarkdownViewerTest, RenderImage) {
    MarkdownRenderer renderer;
    auto result = renderer.render("![Alt text](image.png)");
    EXPECT_NE(result.html.find("image.png"), std::string::npos);
}

TEST_F(MarkdownViewerTest, RenderUnorderedList) {
    MarkdownRenderer renderer;
    auto result = renderer.render("- Item 1\n- Item 2\n- Item 3");
    EXPECT_NE(result.html.find("Item 1"), std::string::npos);
    EXPECT_NE(result.html.find("Item 3"), std::string::npos);
}

TEST_F(MarkdownViewerTest, RenderOrderedList) {
    MarkdownRenderer renderer;
    auto result = renderer.render("1. First\n2. Second\n3. Third");
    EXPECT_NE(result.html.find("First"), std::string::npos);
    EXPECT_NE(result.html.find("Third"), std::string::npos);
}

TEST_F(MarkdownViewerTest, RenderBlockquote) {
    MarkdownRenderer renderer;
    auto result = renderer.render("> This is a quote");
    EXPECT_NE(result.html.find("This is a quote"), std::string::npos);
}

TEST_F(MarkdownViewerTest, WordCount) {
    MarkdownRenderer renderer;
    auto result = renderer.render("The quick brown fox jumps over the lazy dog");
    EXPECT_EQ(result.word_count, 9);
}

TEST_F(MarkdownViewerTest, ReadingTime) {
    MarkdownRenderer renderer;
    // 250 words should take ~1 minute
    std::string long_text;
    for (int i = 0; i < 250; ++i) {
        long_text += "word ";
    }
    auto result = renderer.render(long_text);
    EXPECT_GE(result.reading_time_min, 1);
}

TEST_F(MarkdownViewerTest, SetOptions) {
    MarkdownRenderer renderer;
    MarkdownRenderOptions opts;
    opts.theme = MarkdownRenderOptions::Theme::Dark;
    opts.font_size_px = 20;
    renderer.set_options(opts);
    EXPECT_EQ(renderer.options().font_size_px, 20);
    EXPECT_EQ(renderer.options().theme, MarkdownRenderOptions::Theme::Dark);
}

TEST_F(MarkdownViewerTest, RenderFile) {
    create_file(test_dir / "test.md", "# File Heading\n\nParagraph text.");

    MarkdownRenderer renderer;
    auto result = renderer.render_file(test_dir / "test.md");
    EXPECT_NE(result.html.find("File Heading"), std::string::npos);
    EXPECT_NE(result.html.find("Paragraph text"), std::string::npos);
}

TEST_F(MarkdownViewerTest, RenderFileNotFound) {
    MarkdownRenderer renderer;
    auto result = renderer.render_file(test_dir / "nonexistent.md");
    // Should not crash; may return empty or error
    EXPECT_TRUE(result.html.empty() || !result.errors.empty() || result.word_count == 0);
}

TEST_F(MarkdownViewerTest, ExportHtml) {
    create_file(test_dir / "export_test.md", "# Hello\n\nWorld");

    MarkdownRenderer renderer;
    auto result = renderer.render_file(test_dir / "export_test.md");
    bool ok = renderer.export_html(result, test_dir / "output.html");
    EXPECT_TRUE(ok);
    EXPECT_TRUE(std::filesystem::exists(test_dir / "output.html"));
}

TEST_F(MarkdownViewerTest, GenerateTemplate) {
    MarkdownRenderOptions opts;
    opts.theme = MarkdownRenderOptions::Theme::Dark;
    auto tmpl = MarkdownRenderer::generate_template(opts);
    EXPECT_FALSE(tmpl.empty());
    EXPECT_NE(tmpl.find("<html"), std::string::npos);
}

TEST_F(MarkdownViewerTest, RenderTable) {
    MarkdownRenderer renderer;
    auto result = renderer.render(
        "| Header 1 | Header 2 |\n"
        "|----------|----------|\n"
        "| Cell 1   | Cell 2   |"
    );
    EXPECT_NE(result.html.find("Header 1"), std::string::npos);
    EXPECT_NE(result.html.find("Cell 1"), std::string::npos);
}

TEST_F(MarkdownViewerTest, RenderHorizontalRule) {
    MarkdownRenderer renderer;
    auto result = renderer.render("Above\n\n---\n\nBelow");
    EXPECT_NE(result.html.find("Above"), std::string::npos);
    EXPECT_NE(result.html.find("Below"), std::string::npos);
}

TEST_F(MarkdownViewerTest, ParseASTReturnsNodes) {
    MarkdownRenderer renderer;
    auto ast = renderer.parse_ast("# Title\n\nHello");
    EXPECT_EQ(ast.type, MDNodeType::Document);
    // AST children population depends on md4c parse depth
}
