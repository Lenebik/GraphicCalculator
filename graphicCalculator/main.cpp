#include <QApplication>

#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    app.setApplicationName("GraphicCalculator");
    app.setApplicationDisplayName(QStringLiteral("Графический калькулятор функций"));

    MainWindow window;
    window.show();

    return app.exec();
}
