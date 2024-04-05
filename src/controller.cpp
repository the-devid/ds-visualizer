#include "controller.h"

#include "public.h"
#include "two_three_tree.h"

namespace NVis {

void Controller::OnInsertButtonClick(Key x) {
    if (model_) {
        model_->Insert(x);
    }
}
void Controller::OnEraseButtonClick(Key x) {
    if (model_) {
        model_->Erase(x);
    }
}
void Controller::OnSearchButtonClick(Key x) {
    if (model_) {
        model_->Contains(x);
    }
}

} // namespace NVis
