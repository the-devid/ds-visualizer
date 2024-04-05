#include "view.h"

#include "public.h"
#include "src/observer.h"
#include "tree_action.h"

#include <QEventLoop>
#include <QGraphicsView>
#include <QTimer>

namespace NVis {

View::View() : port_([]() {}, [this](TreeActionsBatch changes) { this->HandleNotification(changes); }, []() {}) {}

void View::HandleNotification(TreeActionsBatch changes) {
    for (const auto& change : changes) {
        if (change.action == ENodeAction::StartQuery) {
            assert(changes.size() == 1 && "Incorrect batch with StartQuery action");
            // Note: this case is significant for async animation. It doesn't matter while animation happens in
            // signal-handler (being a long-time operation), so we simply check a correctness of an operation.
            assert(storage_.empty());
            return;
        }
        if (change.action == ENodeAction::EndQuery) {
            assert(changes.size() == 1 && "Incorrect batch with EndQuery action");
            AnimateQueries();
            return;
        }
    }
    storage_.emplace_back(changes);
}

Observer<TreeActionsBatch>* View::GetPort() {
    return &port_;
}

void View::AnimateQueries() {
    for (const auto& changes_batch : storage_) {
        for (const auto& change : changes_batch) {
            scene_.clear();
            scene_.addText("Change!");
            QEventLoop loop;
            QTimer t;
            t.setSingleShot(true);
            QTimer::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
            t.start(100);
            loop.exec();
        }
    }
    scene_.clear();
    scene_.addText("Ended Changes!");
    storage_.clear();
}

} // namespace NVis
