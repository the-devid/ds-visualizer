#include "application.h"

#include "public.h"
#include "src/view.h"
#include "src/window.h"

#include <QMessageBox>

namespace NVis {

Application& Application::Instance() {
    static Application app;
    return app;
}

Application::Application()
    : view_(),
      window_(),
      model_(),
      controller_(&model_, window_.key_edit_, window_.insert_button_, window_.erase_button_, window_.search_button_) {

    QObject::connect(window_.insert_button_, &QPushButton::clicked, &controller_, &Controller::OnInsertButtonClick);
    QObject::connect(window_.erase_button_, &QPushButton::clicked, &controller_, &Controller::OnEraseButtonClick);
    QObject::connect(window_.search_button_, &QPushButton::clicked, &controller_, &Controller::OnSearchButtonClick);

    model_.GetPort()->Subscribe(view_.GetPort());
    window_.view_->setScene(&view_.scene_);
}

} // namespace NVis
