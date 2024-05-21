#pragma once

#include "observer.h"
#include "tree_action.h"
#include "tree_drawing_model.h"

#include <QTimer>

#include <queue>

namespace NVis {

class AnimationProducer {
public:
    AnimationProducer(TreeDrawingModel* drawing_model);
    Observer<TreeActionsBatch>* GetTreeActionsPort();

private:
    void HandleNotification(const TreeActionsBatch& actions);
    //! Draws animation of all the stored changes in Model frame by frame using a call to drawing model and calling
    //! itself with `QTimer`. This animation "loop" can be cancelled by `HandleNotification`.
    void AnimateQueries();
    void FinishAnimationImmediately();

    static constexpr int kDelayBetweenFrames = 300;

    Observer<TreeActionsBatch> port_;
    std::queue<TreeActionsBatch> storage_;
    QTimer animation_timer_;
    TreeDrawingModel* drawing_model_;
};

} // namespace NVis
