#include <QApplication>
#include <QWebEngineView>
#include <QWebChannel>
#include <QUrl>
#include <QDir>
#include <QWebEngineSettings>
#include "InTeX/evaluator.hpp"
#include "InTeX/lexer.hpp"
#include "InTeX/parser.hpp"
#include "bridge.hpp"

int main(int argc, char *argv[]) {
    // Initialize the Qt application
    QApplication app(argc, argv);
    Bridge bridge;
    QWebChannel channel;
    channel.registerObject(QStringLiteral("bridge"), &bridge);

    // Create the WebEngine view that will render HTML content
    QWebEngineView view;
    view.page()->setWebChannel(&channel);
    view.settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);

    // Set the URL to the HTML page using the absolute path of the current directory
    QDir dir(QCoreApplication::applicationDirPath());
    QUrl url = QUrl::fromLocalFile(dir.filePath("../src/web/index.html"));
    view.setUrl(url);

    // Set the size of the window
    view.resize(800, 600);

    // Show the window
    view.show();

    // Execute the application
    return app.exec();
}