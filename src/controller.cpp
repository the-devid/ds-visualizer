#include "controller.h"

#include "public.h"
#include "two_three_tree.h"

namespace NVis {

void Controller::OnInsertButtonClick(Key x) {
    SetButtonsState(/*enabled=*/false);
    if (model_) {
        model_->Insert(x);
    }
    SetButtonsState(/*enabled=*/true);
}
void Controller::OnEraseButtonClick(Key x) {
    SetButtonsState(/*enabled=*/false);
    if (model_) {
        model_->Erase(x);
    }
    SetButtonsState(/*enabled=*/true);
}
void Controller::OnSearchButtonClick(Key x) {
    SetButtonsState(/*enabled=*/false);
    if (model_) {
        model_->Contains(x);
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
