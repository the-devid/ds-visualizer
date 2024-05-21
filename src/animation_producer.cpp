#include "animation_producer.h"

namespace NVis {

AnimationProducer::AnimationProducer(TreeDrawingModel* drawing_model)
    : port_([this](const TreeActionsBatch& changes) { this->HandleNotification(changes); },
            [this](const TreeActionsBatch& changes) { this->HandleNotification(changes); }, []() {}),
      animation_timer_(),
      drawing_model_(drawing_model) {
    animation_timer_.setSingleShot(true);
    QObject::connect(&animation_timer_, &QTimer::timeout, [this]() { this->AnimateQueries(); });
}

Observer<TreeActionsBatch>* AnimationProducer::GetTreeActionsPort() {
    return &port_;
}

void AnimationProducer::HandleNotification(const TreeActionsBatch& actions) {
    // TODO: rewrite this in few `assert(std::find_if(...) == ...)`
    for (ssize_t action_ind = 0; action_ind < std::ssize(actions); ++action_ind) {
        const auto& action = actions[action_ind];
        if (action.action_type == ENodeAction::StartQuery) {
            assert(action_ind == 0 && "Garbage before StartQuery action");
            FinishAnimationImmediately();
        }
        if (action.action_type == ENodeAction::EndQuery) {
            assert(action_ind + 1 == std::ssize(actions) && "Garbage after EndQuery action");
        }
    }
    storage_.emplace(actions);
    if (actions.back().action_type == ENodeAction::EndQuery) {
        AnimateQueries();
    }
}

void AnimationProducer::AnimateQueries() {
    if (drawing_model_) {
        drawing_model_->DrawActions(storage_.front());
    }
    storage_.pop();
    if (!storage_.empty()) {
        animation_timer_.start(kDelayBetweenFrames);
    }
}

void AnimationProducer::FinishAnimationImmediately() {
    while (!storage_.empty()) {
        if (drawing_model_) {
            drawing_model_->DrawActions(storage_.front());
        }
        storage_.pop();
    }
    if (animation_timer_.isActive()) {
        animation_timer_.stop();
    }
}

} // namespace NVis
