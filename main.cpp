#include "src/application.h"
#include <QApplication>

int main(int argc, char** argv) {
    QApplication qt_app(argc, argv);
    NVis::Application::Instance();
    return qt_app.exec();
}
