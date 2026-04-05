#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

#include "FileModel.h"
#include "TreemapModel.h"
#include "NativeMarkdownView.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // bobfilez-specific bridges remain registered in their own namespaces.
    // BTK retargeting removes the previous BobUI-specific QML registration step;
    // remaining native QML bridges are registered locally here.
    qmlRegisterType<FileModel>("Omni.File", 1, 0, "FileModel");
    qmlRegisterType<fo::gui::TreemapModel>("Omni.Viz", 1, 0, "TreemapModel");
    qmlRegisterType<fo::gui::NativeMarkdownView>("Omni.Native", 1, 0, "MarkdownView");

    QQmlApplicationEngine engine;
    const QUrl main_qml(QStringLiteral("qrc:/main.qml"));

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [main_qml](QObject* object, const QUrl& object_url) {
            if (!object && object_url == main_qml) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(main_qml);
    return app.exec();
}
