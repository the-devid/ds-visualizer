#include "controller.h"

#include "public.h"
#include "two_three_tree.h"

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

Controller::Controller(Model* model, QLineEdit* key_edit, QPushButton* insert_button, QPushButton* erase_button,
                       QPushButton* search_button)
    : model_(model),
      key_edit_(key_edit),
      insert_button_(insert_button),
      erase_button_(erase_button),
      search_button_(search_button) {}

void Controller::OnInsertButtonClick() {
    SetButtonsState(/*enabled=*/false);
    HandleInsertRequest();
    SetButtonsState(/*enabled=*/true);
}

void Controller::OnEraseButtonClick() {
    SetButtonsState(/*enabled=*/false);
    HandleEraseRequest();
    SetButtonsState(/*enabled=*/true);
}

void Controller::OnSearchButtonClick() {
    SetButtonsState(/*enabled=*/false);
    HandleSearchRequest();
    SetButtonsState(/*enabled=*/true);
}

void Controller::HandleInsertRequest() const {
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

void Controller::HandleEraseRequest() const {
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

void Controller::HandleSearchRequest() const {
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

void Controller::SetButtonsState(bool enabled) const {
    if (insert_button_) {
        insert_button_->setEnabled(enabled);
    }
    if (erase_button_) {
        erase_button_->setEnabled(enabled);
    }
    if (search_button_) {
        search_button_->setEnabled(enabled);
    }
}

} // namespace NVis
