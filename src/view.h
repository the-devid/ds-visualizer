#pragma once

#include "observer.h"
#include "tree_action.h"

#include <QGraphicsScene>
#include <QTimer>

#include <queue>

namespace NVis {

class DrawingInfo {
    class DrawingInfoImpl;

public:
    DrawingInfo();
    ~DrawingInfo();

    void DrawActions(const TreeActionsBatch& actions);
    QGraphicsScene* GetScenePort();

private:
    std::unique_ptr<DrawingInfoImpl> impl_;
    QGraphicsScene scene_;
};

class View {
public:
    View();

    Observer<TreeActionsBatch>* GetTreeActionsPort();
    QGraphicsScene* GetGraphicsModelPort();

private:
    void HandleNotification(const TreeActionsBatch& actions);

    //! Draws animation of all the stored changes in Model frame by frame using a call to drawing model and calling
    //! itself with `QTimer`. This animation "loop" can be cancelled by `HandleNotification`.
    void AnimateQueries();

    static constexpr int kDelayBetweenFrames = 300;
    std::unique_ptr<DrawingInfo> drawing_info_;
    Observer<TreeActionsBatch> port_;
    std::queue<TreeActionsBatch> storage_;
    QTimer animation_timer_;
};

} // namespace NVis
