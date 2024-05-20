#include "window.h"

namespace NVis {

Window::Window()
    : QMainWindow(),
      view_(new QGraphicsView(this)),
      key_edit_(new QLineEdit(this)),
      insert_button_(new QPushButton("Insert", this)),
      erase_button_(new QPushButton("Erase", this)),
      search_button_(new QPushButton("Search", this)) {

    auto central_widget = new QWidget(this);
    setCentralWidget(central_widget);
    auto layout = new QGridLayout;
    centralWidget()->setLayout(layout);

    layout->addWidget(view_, 0, 0, 0, -1);
    layout->addWidget(key_edit_, 1, 0, 1, -1);
    layout->addWidget(insert_button_, 2, 0);
    layout->addWidget(erase_button_, 2, 1);
    layout->addWidget(search_button_, 2, 2);
    setMinimumWidth(kWidth);
    setMinimumHeight(kHeight);
}

void Window::SubscribeViewWidgetTo(QGraphicsScene* scene) {
    view_->setScene(scene);
}

QLineEdit* Window::GetKeyEdit() {
    return key_edit_;
}

QPushButton* Window::GetInsertButton() {
    return insert_button_;
}

QPushButton* Window::GetEraseButton() {
    return erase_button_;
}

QPushButton* Window::GetSearchButton() {
    return search_button_;
}

} // namespace NVis
