#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle(QString::fromLatin1("bobfilez BTK Demo"));
    window.resize(720, 480);

    auto *layout = new QVBoxLayout(&window);
    layout->addWidget(new QLabel(QString::fromLatin1("bobfilez BTK / CopperSpice demo frontend"), &window));
    layout->addWidget(new QLabel(QString::fromLatin1("This target is intentionally lightweight so the BTK lane can stay buildable even while declarative/QML remains a separate research track."), &window));

    window.show();
    return app.exec();
}
