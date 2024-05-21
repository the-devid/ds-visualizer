#pragma once

#include "observer.h"
#include "tree_action.h"

#include <QGraphicsScene>
#include <QTimer>

#include <queue>

namespace NVis {

class TreeDrawingModel {
    class TreeDrawingModelImpl;

public:
    TreeDrawingModel();
    ~TreeDrawingModel();

    void DrawActions(const TreeActionsBatch& actions);
    QGraphicsScene* GetScenePort();

private:
    std::unique_ptr<TreeDrawingModelImpl> impl_;
    QGraphicsScene scene_;
};

class AnimationProducer {
    static constexpr int kDelayBetweenFrames = 300;

public:
    AnimationProducer(TreeDrawingModel* drawing_model);
    Observer<TreeActionsBatch>* GetTreeActionsPort();

private:
    void HandleNotification(const TreeActionsBatch& actions);
    //! Draws animation of all the stored changes in Model frame by frame using a call to drawing model and calling
    //! itself with `QTimer`. This animation "loop" can be cancelled by `HandleNotification`.
    void AnimateQueries();
    void FinishAnimationImmediately();

    Observer<TreeActionsBatch> port_;
    std::queue<TreeActionsBatch> storage_;
    QTimer animation_timer_;
    TreeDrawingModel* drawing_model_;
};

} // namespace NVis
