#include "application.h"

#include "public.h"

#include <QMessageBox>

namespace NVis {

Application& Application::Instance() {
    static Application app;
    return app;
}

Application::Application() {
    QObject::connect(window_.insert_button_, &QPushButton::clicked, &controller_, [this]() {
        auto input = window_.key_edit_->text();
        window_.key_edit_->clear();
        controller_.OnInsertButtonClick(std::move(input));
    });
    QObject::connect(window_.erase_button_, &QPushButton::clicked, &controller_, [this]() {
        auto input = window_.key_edit_->text();
        window_.key_edit_->clear();
        controller_.OnEraseButtonClick(std::move(input));
    });
    QObject::connect(window_.search_button_, &QPushButton::clicked, &controller_, [this]() {
        auto input = window_.key_edit_->text();
        window_.key_edit_->clear();
        controller_.OnSearchButtonClick(std::move(input));
    });
    controller_.insert_button_ = window_.insert_button_;
    controller_.erase_button_ = window_.erase_button_;
    controller_.search_button_ = window_.search_button_;

    controller_.model_ = &model_;
    model_.GetPort()->Subscribe(view_.GetPort());

    window_.view_->setScene(&view_.scene_);
}

Application::~Application() = default;

} // namespace NVis
