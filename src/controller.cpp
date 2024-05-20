#include "controller.h"

#include <QMessageBox>

#include <optional>

namespace NVis {

namespace {
void ShowIncorrectInputMessage() {
    QMessageBox error_box;
    error_box.setText("Input contains not a valid number!");
    error_box.exec();
}
} // namespace

Controller::Controller(Model* model, QLineEdit* key_edit) : model_(model), key_edit_(key_edit) {}

void Controller::OnInsertButtonClick() {
    if (!model_) {
        return;
    }
    auto maybe_key = TryGetKeyFromEdit();
    if (!maybe_key) {
        ShowIncorrectInputMessage();
    } else {
        model_->Insert(*maybe_key);
    }
}

void Controller::OnEraseButtonClick() {
    if (!model_) {
        return;
    }
    auto maybe_key = TryGetKeyFromEdit();
    if (!maybe_key) {
        ShowIncorrectInputMessage();
    } else {
        model_->Erase(*maybe_key);
    }
}

void Controller::OnSearchButtonClick() {
    if (!model_) {
        return;
    }
    auto maybe_key = TryGetKeyFromEdit();
    if (!maybe_key) {
        ShowIncorrectInputMessage();
    } else {
        model_->Contains(*maybe_key);
    }
}

std::optional<Key> Controller::TryGetKeyFromEdit() const {
    if (!key_edit_) {
        return std::nullopt;
    }
    bool is_correct_value;
    auto input = key_edit_->text();
    key_edit_->setText("");
    Key key = input.toInt(&is_correct_value);
    if (is_correct_value) {
        return key;
    } else {
        return std::nullopt;
    }
}

} // namespace NVis
