#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "OmniApp.h"
#include "FileModel.h"

int main(int argc, char *argv[])
{
    OmniApplication app(argc, argv);
    app.initializeJuce();

    qmlRegisterType<FileModel>("Omni.File", 1, 0, "FileModel");

    FileModel fileModel;
    
    // Inject the model into the QML context
    // In a real OmniUI app, this might be handled by OmniApp
    
    app.loadMainSource("assets/main.qml");

    return app.exec();
}
