#include "NativeMarkdownView.h"

#include <QPainter>
#include <QTextOption>
#include <QWheelEvent>

#include <algorithm>
#include <sstream>

namespace {
QString first_heading_or_fallback(const QString& markdown) {
    const QStringList lines = markdown.split('\n');
    for (const QString& line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith("#")) {
            QString heading = trimmed;
            while (heading.startsWith('#')) heading.remove(0, 1);
            heading = heading.trimmed();
            if (!heading.isEmpty()) return heading;
        }
    }
    return QStringLiteral("Markdown Preview");
}

QString theme_background(int mode) {
    switch (mode) {
    case 0: return QStringLiteral("#ffffff");
    case 2: return QStringLiteral("#f4e8d0");
    default: return QStringLiteral("#1e1e1e");
    }
}

QString theme_foreground(int mode) {
    switch (mode) {
    case 0: return QStringLiteral("#202020");
    case 2: return QStringLiteral("#3a2f1f");
    default: return QStringLiteral("#d4d4d4");
    }
}

QString theme_code_background(int mode) {
    switch (mode) {
    case 0: return QStringLiteral("#f5f5f5");
    case 2: return QStringLiteral("#eadfc8");
    default: return QStringLiteral("#111111");
    }
}
} // namespace

namespace fo::gui {

NativeMarkdownView::NativeMarkdownView(QQuickItem* parent)
    : QQuickPaintedItem(parent) {
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setFlag(QQuickItem::ItemHasContents, true);

    QTextOption option;
    option.setWrapMode(QTextOption::WordWrap);
    document_.setDefaultTextOption(option);

    rebuildDocument();
}

void NativeMarkdownView::paint(QPainter* painter) {
    painter->fillRect(boundingRect(), QColor(theme_background(theme_mode_)));

    painter->save();
    constexpr qreal padding = 20.0;
    painter->translate(padding, padding - scroll_y_);
    document_.drawContents(painter);
    painter->restore();
}

void NativeMarkdownView::setSourceMarkdown(const QString& markdown) {
    if (source_markdown_ == markdown) return;
    source_markdown_ = markdown;
    emit sourceMarkdownChanged();
    rebuildDocument();
}

void NativeMarkdownView::setThemeMode(int mode) {
    if (theme_mode_ == mode) return;
    theme_mode_ = mode;
    emit themeModeChanged();
    rebuildDocument();
}

void NativeMarkdownView::setFontFamily(const QString& family) {
    if (font_family_ == family) return;
    font_family_ = family;
    emit fontFamilyChanged();
    rebuildDocument();
}

void NativeMarkdownView::setFontPixelSize(int size) {
    if (font_pixel_size_ == size) return;
    font_pixel_size_ = size;
    emit fontPixelSizeChanged();
    rebuildDocument();
}

void NativeMarkdownView::scrollToTop() {
    if (scroll_y_ == 0.0) return;
    scroll_y_ = 0.0;
    update();
}

void NativeMarkdownView::scrollToAnchor(const QString&) {
    // Native QTextDocument preview does not currently map markdown headings to
    // DOM anchors. Keep the API so QML callers don't break; future work can map
    // heading offsets into scroll positions if/when richer outlining is needed.
}

void NativeMarkdownView::wheelEvent(QWheelEvent* event) {
    constexpr qreal k_scroll_step = 48.0;
    scroll_y_ -= event->angleDelta().y() / 120.0 * k_scroll_step;
    clampScroll();
    update();
    event->accept();
}

void NativeMarkdownView::geometryChange(const QRectF& new_geometry, const QRectF& old_geometry) {
    QQuickPaintedItem::geometryChange(new_geometry, old_geometry);
    if (!qFuzzyCompare(new_geometry.width(), old_geometry.width())) {
        rebuildDocument();
    }
}

void NativeMarkdownView::rebuildDocument() {
    fo::core::MarkdownRenderer renderer(buildOptions());
    const auto result = renderer.render(source_markdown_.toStdString());

    word_count_ = result.word_count;
    reading_time_minutes_ = result.reading_time_min;
    document_title_ = result.frontmatter.title.empty()
        ? first_heading_or_fallback(source_markdown_)
        : QString::fromStdString(result.frontmatter.title);

    document_.setDefaultStyleSheet(QString());
    document_.setHtml(buildStyledHtml(QString::fromStdString(result.body_html)));

    constexpr qreal padding = 40.0;
    const qreal usable_width = std::max<qreal>(100.0, width() - padding);
    document_.setTextWidth(usable_width);

    const qreal new_content_height = document_.size().height() + padding;
    if (!qFuzzyCompare(content_height_, new_content_height)) {
        content_height_ = new_content_height;
        emit contentHeightChanged();
    }

    clampScroll();
    emit statsChanged();
    update();
}

QString NativeMarkdownView::buildStyledHtml(const QString& body_html) const {
    const QString bg = theme_background(theme_mode_);
    const QString fg = theme_foreground(theme_mode_);
    const QString code_bg = theme_code_background(theme_mode_);

    return QString(
        "<html><head><style>"
        "body { background:%1; color:%2; font-family:'%3'; font-size:%4px; line-height:1.6; }"
        "a { color:#0078d4; text-decoration:none; }"
        "h1,h2,h3,h4,h5,h6 { color:%2; margin-top:18px; margin-bottom:8px; }"
        "pre { background:%5; padding:10px; border-radius:6px; white-space:pre-wrap; }"
        "code { background:%5; }"
        "blockquote { border-left:4px solid #666; margin-left:0; padding-left:12px; color:#909090; }"
        "table { border-collapse:collapse; width:100%; }"
        "th, td { border:1px solid #555; padding:6px; }"
        "img { max-width:100%; }"
        "</style></head><body>%6</body></html>")
        .arg(bg, fg, font_family_)
        .arg(font_pixel_size_)
        .arg(code_bg, body_html);
}

fo::core::MarkdownRenderOptions NativeMarkdownView::buildOptions() const {
    fo::core::MarkdownRenderOptions options;
    options.font_family = font_family_.toStdString();
    options.font_size_px = font_pixel_size_;
    options.syntax_highlighting = false;
    options.math_inline = false;
    options.math_block = false;
    options.mermaid = false;
    options.highlight_theme = "github";

    switch (theme_mode_) {
    case 0: options.theme = fo::core::MarkdownRenderOptions::Theme::Light; break;
    case 2: options.theme = fo::core::MarkdownRenderOptions::Theme::Sepia; break;
    default: options.theme = fo::core::MarkdownRenderOptions::Theme::Dark; break;
    }

    return options;
}

void NativeMarkdownView::clampScroll() {
    const qreal viewport_height = std::max<qreal>(1.0, height());
    const qreal max_scroll = std::max<qreal>(0.0, content_height_ - viewport_height);
    scroll_y_ = std::clamp(scroll_y_, 0.0, max_scroll);
}

} // namespace fo::gui
