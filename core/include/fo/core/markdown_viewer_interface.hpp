#pragma once
/// @file markdown_viewer_interface.hpp
/// @brief Markdown renderer backend for bobfilez using md4c + cmark.
///
/// Features modeled on:
///   Typora          — WYSIWYG, instant render, math/mermaid/code
///   Mark Text       — open-source Typora alternative
///   Obsidian        — linked notes, backlinks, graph view
///   Zettlr          — academic writing, citations, dark/light themes
///   VSCode Preview  — split-pane with syntax highlighting
///
/// Rendering pipeline:
///   1. md4c or cmark parses Markdown → AST
///   2. HTML renderer generates styled HTML
///   3. QML WebEngineView renders with highlight.js + KaTeX + Mermaid
///
/// Supported extensions:
///   CommonMark          — baseline spec (full)
///   GitHub Flavored MD  — tables, task lists, autolinks, strikethrough
///   Math (LaTeX)        — $inline$ and $$block$$ via KaTeX
///   Mermaid Diagrams    — flowchart, sequence, gantt, pie, etc.
///   Syntax Highlighting — 200+ languages via highlight.js
///   Footnotes           — [^1] footnote syntax
///   Definition Lists    — : term notation
///   Wikilinks           — [[Page Name]] internal links
///   Frontmatter         — YAML frontmatter block
///   Emoji               — :smile: emoji codes
///   Superscript/Sub     — x^2^ and H~2~O

#include <string>
#include <vector>
#include <filesystem>
#include <map>

namespace fo::core {

struct MarkdownRenderOptions {
    // CommonMark dialect options
    bool gfm_tables = true;         // GitHub Flavored Markdown: tables
    bool gfm_task_lists = true;     // [ ] / [x] checkboxes
    bool gfm_autolinks = true;      // Auto-link URLs
    bool gfm_strikethrough = true;  // ~~strikethrough~~
    bool footnotes = true;          // [^1] footnotes
    bool definition_lists = true;   // : term
    bool wikilinks = false;         // [[Page]]
    bool frontmatter = true;        // Parse and strip YAML frontmatter
    bool emoji = true;              // :smile: codes
    bool superscript = true;        // x^2^
    bool subscript = true;          // H~2~O
    bool typographer = true;        // Smart quotes, dashes, ellipsis
    bool math_inline = true;        // $x^2$
    bool math_block = true;         // $$block$$
    bool mermaid = true;            // ```mermaid code blocks → diagrams
    bool syntax_highlighting = true;// ```lang code blocks → highlighted HTML
    std::string highlight_theme = "github"; // highlight.js theme name

    // Rendering style
    enum class Theme { Light, Dark, Sepia, Custom } theme = Theme::Light;
    std::string custom_css;         // Injected into <style> if theme==Custom
    std::string font_family = "system-ui";
    int font_size_px = 16;
    int max_width_px = 800;         // Content column max width (0=fill)
    bool show_toc = false;          // Auto-generate table of contents
    bool number_headings = false;

    // Link handling
    bool open_external_links_in_browser = true;
    std::filesystem::path base_dir; // Resolve relative image/link paths from here

    // Export options
    bool embed_images_base64 = false; // Embed images in HTML as data-URIs (for export)
};

/// AST node types for parsed Markdown
enum class MDNodeType {
    Document, Heading, Paragraph, BlockQuote, List, ListItem,
    CodeBlock, FencedCode, HtmlBlock, ThematicBreak, Table, TableRow, TableCell,
    Text, Emphasis, Strong, Strikethrough, Code, Link, Image, HtmlInline,
    SoftBreak, HardBreak, Math, MathBlock, MermaidBlock, Emoji, Footnote, Frontmatter
};

struct MDNode {
    MDNodeType type;
    std::string text;      // Text content (for Text, Code, etc.)
    std::string attr;      // href for Link, src for Image, language for FencedCode
    std::string title;
    int level = 0;         // For Heading (1-6)
    bool checked = false;  // For ListItem (task list)
    std::vector<MDNode> children;
};

/// Frontmatter metadata extracted from YAML header
struct FrontmatterData {
    std::string title;
    std::string author;
    std::string date;
    std::string description;
    std::vector<std::string> tags;
    std::map<std::string, std::string> extra; // All other key/value pairs
    bool present = false;
};

/// Result of rendering a Markdown document
struct MarkdownRenderResult {
    std::string html;               // Full rendered HTML page (standalone)
    std::string body_html;          // Just the <body> content (for inline embed)
    std::string toc_html;           // Table of contents HTML
    FrontmatterData frontmatter;
    std::vector<std::string> headings; // Ordered heading texts (for TOC)
    std::vector<std::string> links;    // All links found in document
    std::vector<std::filesystem::path> images; // All image paths referenced
    std::vector<std::string> wikilinks; // All [[wikilinks]] found
    int word_count = 0;
    int reading_time_min = 0;       // Estimated reading time (250 wpm)
    std::vector<std::string> errors; // Parser warnings/errors
};

/// The Markdown rendering engine
class MarkdownRenderer {
public:
    explicit MarkdownRenderer(const MarkdownRenderOptions& opts = {});

    /// Render a Markdown string to HTML
    MarkdownRenderResult render(const std::string& markdown_text);

    /// Render a Markdown file to HTML
    MarkdownRenderResult render_file(const std::filesystem::path& path);

    /// Export rendered HTML to a standalone file
    bool export_html(const MarkdownRenderResult& result, const std::filesystem::path& output);

    /// Export to PDF via Pandoc (requires pandoc + wkhtmltopdf or weasyprint)
    bool export_pdf(const std::filesystem::path& md_path, const std::filesystem::path& output);

    /// Export to DOCX via Pandoc
    bool export_docx(const std::filesystem::path& md_path, const std::filesystem::path& output);

    /// Parse document structure only (no HTML generation) — for outlining
    MDNode parse_ast(const std::string& markdown_text);

    /// Update options (e.g. toggle dark mode without re-parsing)
    void set_options(const MarkdownRenderOptions& opts);
    const MarkdownRenderOptions& options() const;

    /// Generate the HTML wrapper (head+style+scripts) for use with WebEngineView
    /// The returned HTML includes KaTeX, Mermaid, and highlight.js from CDN or bundled.
    static std::string generate_template(const MarkdownRenderOptions& opts);

private:
    MarkdownRenderOptions opts_;
    std::string render_node(const MDNode& node, int depth) const;
    static std::string highlight_code(const std::string& code, const std::string& lang);
    static std::string render_math_inline(const std::string& latex);
    static std::string render_math_block(const std::string& latex);
};

} // namespace fo::core
