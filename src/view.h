#pragma once

#include "observer.h"
#include "public.h"

#include <QGraphicsScene>

namespace NVis {

class View {
public:
    View();
    ~View();

    Observer<TreeActionsBatch>* GetTreeActionsPort();
    QGraphicsScene* GetGraphicsModelPort();

private:
    void HandleNotification(const TreeActionsBatch& actions);

    //! Draws animation of all the stored changes in Model frame by frame and clears `storage_`.
    void AnimateQueries();

    //! Kind of pimpl.
    struct DrawingInfo;
    std::unique_ptr<DrawingInfo> drawing_info_ptr_;

    static constexpr int kDelayBetweenFrames = 300;
    Observer<TreeActionsBatch> port_;
    QGraphicsScene scene_;
    std::vector<TreeActionsBatch> storage_;
};

} // namespace NVis
