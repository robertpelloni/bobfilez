#pragma once

#include <QQuickPaintedItem>
#include <QTextDocument>
#include <QString>

#include "fo/core/markdown_viewer_interface.hpp"

namespace fo::gui {

class NativeMarkdownView : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QString sourceMarkdown READ sourceMarkdown WRITE setSourceMarkdown NOTIFY sourceMarkdownChanged)
    Q_PROPERTY(int themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    Q_PROPERTY(int fontPixelSize READ fontPixelSize WRITE setFontPixelSize NOTIFY fontPixelSizeChanged)
    Q_PROPERTY(int wordCount READ wordCount NOTIFY statsChanged)
    Q_PROPERTY(int readingTimeMinutes READ readingTimeMinutes NOTIFY statsChanged)
    Q_PROPERTY(QString documentTitle READ documentTitle NOTIFY statsChanged)
    Q_PROPERTY(qreal contentHeight READ contentHeight NOTIFY contentHeightChanged)

public:
    explicit NativeMarkdownView(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;

    QString sourceMarkdown() const { return source_markdown_; }
    void setSourceMarkdown(const QString& markdown);

    int themeMode() const { return theme_mode_; }
    void setThemeMode(int mode);

    QString fontFamily() const { return font_family_; }
    void setFontFamily(const QString& family);

    int fontPixelSize() const { return font_pixel_size_; }
    void setFontPixelSize(int size);

    int wordCount() const { return word_count_; }
    int readingTimeMinutes() const { return reading_time_minutes_; }
    QString documentTitle() const { return document_title_; }
    qreal contentHeight() const { return content_height_; }

    Q_INVOKABLE void scrollToTop();
    Q_INVOKABLE void scrollToAnchor(const QString& anchor);

signals:
    void sourceMarkdownChanged();
    void themeModeChanged();
    void fontFamilyChanged();
    void fontPixelSizeChanged();
    void statsChanged();
    void contentHeightChanged();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void geometryChange(const QRectF& new_geometry, const QRectF& old_geometry) override;

private:
    void rebuildDocument();
    QString buildStyledHtml(const QString& body_html) const;
    fo::core::MarkdownRenderOptions buildOptions() const;
    void clampScroll();

    QString source_markdown_;
    int theme_mode_ = 1; // 0=light, 1=dark, 2=sepia
    QString font_family_ = "system-ui";
    int font_pixel_size_ = 16;
    int word_count_ = 0;
    int reading_time_minutes_ = 0;
    QString document_title_;
    qreal content_height_ = 0.0;
    qreal scroll_y_ = 0.0;
    QTextDocument document_;
};

} // namespace fo::gui
