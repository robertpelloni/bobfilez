/// @file markdown_viewer.cpp
/// @brief Implementation of MarkdownRenderer using md4c backend.

#include "fo/core/markdown_viewer_interface.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

#include "md4c-html.h"

namespace fo::core {

MarkdownRenderer::MarkdownRenderer(const MarkdownRenderOptions& opts) : opts_(opts) {}

void MarkdownRenderer::set_options(const MarkdownRenderOptions& opts) { opts_ = opts; }
const MarkdownRenderOptions& MarkdownRenderer::options() const { return opts_; }

// Callback for md4c-html output
static void process_output(const MD_CHAR* text, MD_SIZE size, void* userdata) {
    std::string* out = static_cast<std::string*>(userdata);
    out->append(text, size);
}

MarkdownRenderResult MarkdownRenderer::render(const std::string& markdown_text) {
    MarkdownRenderResult result;
    
    // Calculate simple word count (approximate)
    int spaces = 0;
    for (char c : markdown_text) if (std::isspace(c)) spaces++;
    result.word_count = spaces + 1;
    result.reading_time_min = std::max(1, result.word_count / 250);

    // Frontmatter parsing (basic YAML extraction)
    std::string md_body = markdown_text;
    if (opts_.frontmatter && markdown_text.substr(0, 4) == "---\n") {
        size_t end_pos = markdown_text.find("\n---\n", 4);
        if (end_pos != std::string::npos) {
            std::string fm = markdown_text.substr(4, end_pos - 4);
            result.frontmatter.present = true;
            // Extremely basic key: value parser
            std::istringstream iss(fm);
            std::string line;
            while (std::getline(iss, line)) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string key = line.substr(0, colon);
                    std::string val = line.substr(colon + 1);
                    // Trim
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    val.erase(0, val.find_first_not_of(" \t"));
                    val.erase(val.find_last_not_of(" \t") + 1);

                    if (key == "title") result.frontmatter.title = val;
                    else if (key == "author") result.frontmatter.author = val;
                    else if (key == "date") result.frontmatter.date = val;
                    else if (key == "description") result.frontmatter.description = val;
                    else result.frontmatter.extra[key] = val;
                }
            }
            md_body = markdown_text.substr(end_pos + 5);
        }
    }

    // Configure md4c parser flags based on options
    unsigned parser_flags = MD_FLAG_COLLAPSEWHITESPACE;
    if (opts_.gfm_tables) parser_flags |= MD_FLAG_TABLES;
    if (opts_.gfm_task_lists) parser_flags |= MD_FLAG_TASKLISTS;
    if (opts_.gfm_autolinks) parser_flags |= MD_FLAG_AUTOLINKS;
    if (opts_.gfm_strikethrough) parser_flags |= MD_FLAG_STRIKETHROUGH;
    if (opts_.footnotes) parser_flags |= MD_FLAG_FOOTNOTES;
    if (opts_.wikilinks) parser_flags |= MD_FLAG_WIKILINKS;
    if (opts_.math_inline || opts_.math_block) parser_flags |= MD_FLAG_LATEXMATHSPANS;

    unsigned render_flags = MD_HTML_FLAG_SKIP_UTF8_BOM;

    int ret = md_html(md_body.c_str(), static_cast<MD_SIZE>(md_body.size()),
                      process_output, &result.body_html, parser_flags, render_flags);

    if (ret != 0) {
        result.errors.push_back("md4c parsing failed with code " + std::to_string(ret));
    }

    result.html = generate_template(opts_) + result.body_html + "\n</div></body></html>";
    return result;
}

MarkdownRenderResult MarkdownRenderer::render_file(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return render(text);
}

bool MarkdownRenderer::export_html(const MarkdownRenderResult& result, const std::filesystem::path& output) {
    std::ofstream file(output, std::ios::binary);
    if (!file) return false;
    file << result.html;
    return true;
}

bool MarkdownRenderer::export_pdf(const std::filesystem::path& md_path, const std::filesystem::path& output) {
    // Requires pandoc in PATH
    std::string cmd = "pandoc \"" + md_path.string() + "\" -o \"" + output.string() + "\" --pdf-engine=wkhtmltopdf";
    int rc = system(cmd.c_str());
    return rc == 0;
}

bool MarkdownRenderer::export_docx(const std::filesystem::path& md_path, const std::filesystem::path& output) {
    std::string cmd = "pandoc \"" + md_path.string() + "\" -o \"" + output.string() + "\"";
    int rc = system(cmd.c_str());
    return rc == 0;
}

std::string MarkdownRenderer::generate_template(const MarkdownRenderOptions& opts) {
    std::ostringstream html;
    html << "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n";
    
    // Theme CSS
    std::string bg = opts.theme == MarkdownRenderOptions::Theme::Dark ? "#1e1e1e" :
                     opts.theme == MarkdownRenderOptions::Theme::Sepia ? "#f4e8d0" : "#ffffff";
    std::string fg = opts.theme == MarkdownRenderOptions::Theme::Dark ? "#d4d4d4" : "#333333";
    std::string link = "#0078d4";

    html << "<style>\n"
         << "body { background-color: " << bg << "; color: " << fg << "; font-family: " << opts.font_family << ", sans-serif; font-size: " << opts.font_size_px << "px; line-height: 1.6; }\n"
         << "a { color: " << link << "; text-decoration: none; }\n"
         << "a:hover { text-decoration: underline; }\n"
         << "pre { background: " << (opts.theme == MarkdownRenderOptions::Theme::Dark ? "#111" : "#f5f5f5") << "; padding: 10px; border-radius: 5px; overflow-x: auto; }\n"
         << "code { font-family: Consolas, monospace; }\n"
         << "blockquote { border-left: 4px solid #ccc; margin-left: 0; padding-left: 16px; color: #888; }\n"
         << "table { border-collapse: collapse; width: 100%; }\n"
         << "th, td { border: 1px solid #ddd; padding: 8px; }\n"
         << "th { background-color: " << (opts.theme == MarkdownRenderOptions::Theme::Dark ? "#333" : "#f2f2f2") << "; }\n";
    
    if (opts.max_width_px > 0) {
        html << ".content { max-width: " << opts.max_width_px << "px; margin: 0 auto; padding: 20px; }\n";
    } else {
        html << ".content { margin: 0 auto; padding: 20px; }\n";
    }
    
    html << opts.custom_css << "\n</style>\n";

    // highlight.js for syntax highlighting
    if (opts.syntax_highlighting) {
        html << "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/" << opts.highlight_theme << ".min.css\">\n"
             << "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js\"></script>\n"
             << "<script>document.addEventListener('DOMContentLoaded', (event) => { document.querySelectorAll('pre code').forEach((el) => { hljs.highlightElement(el); }); });</script>\n";
    }

    // KaTeX for math
    if (opts.math_inline || opts.math_block) {
        html << "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/katex@0.16.10/dist/katex.min.css\">\n"
             << "<script defer src=\"https://cdn.jsdelivr.net/npm/katex@0.16.10/dist/katex.min.js\"></script>\n"
             << "<script defer src=\"https://cdn.jsdelivr.net/npm/katex@0.16.10/dist/contrib/auto-render.min.js\" onload=\"renderMathInElement(document.body);\"></script>\n";
    }

    // Mermaid for diagrams
    if (opts.mermaid) {
        html << "<script type=\"module\">\n"
             << "import mermaid from 'https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.esm.min.mjs';\n"
             << "mermaid.initialize({ startOnLoad: true, theme: '" << (opts.theme == MarkdownRenderOptions::Theme::Dark ? "dark" : "default") << "' });\n"
             << "</script>\n";
    }

    html << "</head>\n<body>\n<div class=\"content\">\n";
    return html.str();
}

MDNode MarkdownRenderer::parse_ast(const std::string&) {
    // Requires full AST builder callbacks via md_parse
    return MDNode{MDNodeType::Document};
}

} // namespace fo::core
