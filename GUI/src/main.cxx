#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <CustomPlotItem.hxx>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    qmlRegisterType<CustomPlotItem>("CustomControls", 1, 0, "CustomPlot");

    QQmlApplicationEngine engine;

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}