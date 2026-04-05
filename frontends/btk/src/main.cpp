#include <QApplication>
#include "DemoWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DemoWindow window;
    window.setWindowTitle(QString::fromLatin1("bobfilez BTK Demo"));
    window.resize(800, 600);
    window.show();

    return app.exec();
}
