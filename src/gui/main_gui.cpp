#include "mainwindow.h"
#include "core/secret_detector.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // регистрация типов для Qt meta system
    qRegisterMetaType<ScanResult>("ScanResult");
    qRegisterMetaType<ScanStatistics>("ScanStatistics");

    // главное окно программы
    MainWindow window;
    window.show();

    return app.exec();
}
