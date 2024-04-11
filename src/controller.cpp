#include "controller.h"

#include "public.h"
#include "two_three_tree.h"

#include <QMessageBox>

namespace NVis {

void Controller::OnInsertButtonClick(QString input) {
    SetButtonsState(/*enabled=*/false);
    if (model_) {
        bool is_correct_value;
        Key key = input.toInt(&is_correct_value);
        if (!is_correct_value) {
            QMessageBox error_box;
            error_box.setText("Input contains not a valid number!");
            error_box.exec();
        } else {
            model_->Insert(key);
        }
    }
    SetButtonsState(/*enabled=*/true);
}
void Controller::OnEraseButtonClick(QString input) {
    SetButtonsState(/*enabled=*/false);
    if (model_) {
        bool is_correct_value;
        Key key = input.toInt(&is_correct_value);
        if (!is_correct_value) {
            QMessageBox error_box;
            error_box.setText("Input contains not a valid number!");
            error_box.exec();
        } else {
            model_->Erase(key);
        }
    }
    SetButtonsState(/*enabled=*/true);
}
void Controller::OnSearchButtonClick(QString input) {
    SetButtonsState(/*enabled=*/false);
    if (model_) {
        bool is_correct_value;
        Key key = input.toInt(&is_correct_value);
        if (!is_correct_value) {
            QMessageBox error_box;
            error_box.setText("Input contains not a valid number!");
            error_box.exec();
        } else {
            model_->Contains(key);
        }
    }
    SetButtonsState(/*enabled=*/true);
}

void Controller::SetButtonsState(bool enabled) {
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
