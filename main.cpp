#include "src/application.h"
#include <QApplication>
#include <QWindow>

int main(int argc, char** argv) {
    QApplication qt_app(argc, argv);
    NVis::Application::Instance();
    return qt_app.exec();
}
