#include "src/application.h"
#include <QApplication>

int main(int argc, char** argv) {
    QApplication qt_runtime(argc, argv);
    NVis::Application app;
    app.ShowWindow();
    return qt_runtime.exec();
}
