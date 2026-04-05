#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <OmniQmlRegistration.h>
#include <fo/core/version.hpp>
#include "QmlEngineWrapper.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    OmniUI::registerQmlTypes();

    QmlEngineWrapper foEngine;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("bobfilezVersion", QString::fromUtf8(fo::core::FO_VERSION.data()));
    engine.rootContext()->setContextProperty("foEngine", &foEngine);

    const QUrl main_url(QStringLiteral("qrc:/frontends/bobui/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                     [main_url](QObject *object, const QUrl &object_url) {
                         if (!object && object_url == main_url)
                             QCoreApplication::exit(-1);
                     },
                     Qt::QueuedConnection);

    engine.load(main_url);
    return app.exec();
}
