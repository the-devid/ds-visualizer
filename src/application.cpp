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
        bool is_correct_value;
        int key = window_.key_edit_->text().toInt(&is_correct_value);
        this->window_.key_edit_->clear();
        if (!is_correct_value) {
            QMessageBox error_box;
            error_box.setText("Input contains not a valid number!");
            error_box.exec();
        } else {
            controller_.OnInsertButtonClick(key);
        }
    });
    QObject::connect(window_.erase_button_, &QPushButton::clicked, &controller_, [this]() {
        bool is_correct_value;
        int key = window_.key_edit_->text().toInt(&is_correct_value);
        this->window_.key_edit_->clear();
        if (!is_correct_value) {
            QMessageBox error_box;
            error_box.setText("Input contains not a valid number!");
            error_box.exec();
        } else {
            controller_.OnEraseButtonClick(key);
        }
    });
    QObject::connect(window_.search_button_, &QPushButton::clicked, &controller_, [this]() {
        bool is_correct_value;
        int key = window_.key_edit_->text().toInt(&is_correct_value);
        this->window_.key_edit_->clear();
        if (!is_correct_value) {
            QMessageBox error_box;
            error_box.setText("Input contains not a valid number!");
            error_box.exec();
        } else {
            controller_.OnSearchButtonClick(key);
        }
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
