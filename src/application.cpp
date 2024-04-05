#include "application.h"

#include "public.h"

namespace NVis {

Application& Application::Instance() {
    static Application app;
    return app;
}

Application::Application() {
    QObject::connect(window_.insert_button_, &QPushButton::clicked, &controller_,
                     [this]() { controller_.OnInsertButtonClick(window_.key_edit_->text().toInt()); });
    QObject::connect(window_.erase_button_, &QPushButton::clicked, &controller_,
                     [this]() { controller_.OnEraseButtonClick(window_.key_edit_->text().toInt()); });
    QObject::connect(window_.search_button_, &QPushButton::clicked, &controller_,
                     [this]() { controller_.OnSearchButtonClick(window_.key_edit_->text().toInt()); });
    controller_.insert_button_ = window_.insert_button_;
    controller_.erase_button_ = window_.erase_button_;
    controller_.search_button_ = window_.search_button_;

    controller_.model_ = &model_;
    model_.GetPort()->Subscribe(view_.GetPort());

    window_.view_->setScene(&view_.scene_);
}

Application::~Application() {
    // Detach scene from view because scene and view are living in a different storage models and can't be toghether,
    // what an unhappy story.
    view_.scene_.views().detach();
    // But getting SIGSEGV on exitting the application was much worse... Don't trust static I guess.
}

} // namespace NVis
