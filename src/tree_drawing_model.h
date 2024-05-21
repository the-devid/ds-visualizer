#pragma once

#include "tree_action.h"

#include <QGraphicsScene>

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

} // namespace NVis
