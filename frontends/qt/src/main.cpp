#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

#include <fo/core/version.hpp>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle(QStringLiteral("bobfilez Qt Demo"));
    window.resize(960, 640);

    auto *central = new QWidget(&window);
    auto *layout = new QVBoxLayout(central);

    auto *title = new QLabel(QStringLiteral("bobfilez Qt GUI Demo"), central);
    title->setStyleSheet(QStringLiteral("font-size: 26px; font-weight: bold;"));

    auto *body = new QLabel(
        QStringLiteral("This minimal frontend proves a plain Qt Widgets shell can coexist with the BobUI, BTK, JUCE, BobGUI, and web lanes."),
        central);
    body->setWordWrap(true);

    auto *version = new QLabel(QStringLiteral("Core version: %1").arg(QString::fromUtf8(fo::core::FO_VERSION.data())), central);

    layout->addWidget(title);
    layout->addWidget(body);
    layout->addWidget(version);
    layout->addStretch(1);

    window.setCentralWidget(central);
    window.statusBar()->showMessage(QStringLiteral("Qt frontend bootstrap ready"));
    window.show();

    return app.exec();
}
