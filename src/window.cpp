#include "window.h"

namespace NVis {

Window::Window(QWidget* parent)
    : QMainWindow(parent),
      layout_(new QGridLayout),
      view_(new QGraphicsView(this)),
      key_edit_(new QLineEdit(this)),
      insert_button_(new QPushButton("Insert", this)),
      erase_button_(new QPushButton("Erase", this)),
      search_button_(new QPushButton("Search", this)) {

    layout_->addWidget(view_, 0, 0, 0, -1);
    layout_->addWidget(key_edit_, 1, 0, 1, -1);
    layout_->addWidget(insert_button_, 2, 0);
    layout_->addWidget(erase_button_, 2, 1);
    layout_->addWidget(search_button_, 2, 2);
    QWidget* central_widget = new QWidget(this);
    setCentralWidget(central_widget);
    centralWidget()->setLayout(layout_);
    setMinimumWidth(width_);
    setMinimumHeight(height_);
    show();
}

} // namespace NVis
