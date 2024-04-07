#pragma once

#include "observer.h"
#include "public.h"
#include "tree_action.h"

#include <QGraphicsScene>

namespace NVis {

class View {
public:
    View();
    void HandleNotification(TreeActionsBatch changes);
    Observer<TreeActionsBatch>* GetPort();

private:
    Observer<TreeActionsBatch> port_;
    QGraphicsScene scene_;
    std::vector<TreeActionsBatch> storage_;

    //! Draws animation of all the stored changes in Model frame by frame and clears `storage_`.
    void AnimateQueries();

    struct DrawingInfo;
    //! Kind of pimpl.
    std::unique_ptr<DrawingInfo> animation_info_ptr_;

    friend class Application;
};

} // namespace NVis
