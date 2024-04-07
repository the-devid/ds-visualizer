#include "view.h"

#include "public.h"
#include "src/observer.h"
#include "tree_action.h"

#include <QColor>
#include <QEventLoop>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QPen>
#include <QRect>
#include <QTimer>

#include <memory>
#include <unordered_map>

namespace NVis {

struct View::DrawingInfo {
    static constexpr int kCellWidth = 50;
    static constexpr int kCellHeight = 30;
    static constexpr int kVerticalMargin = 50;
    static constexpr int kHorizontalMargin = 50;

    struct NodeForDraw {
        std::vector<Key> keys;
        std::vector<std::unique_ptr<NodeForDraw>> children;
        QColor background_color = QColorConstants::White;
    };
    std::unique_ptr<NodeForDraw> root;
    std::unordered_map<MemoryAddress, NodeForDraw*> address_to_node;
    std::vector<ssize_t> node_count_on_height;
    std::vector<ssize_t> key_count_on_height;
    std::vector<NodeForDraw> painted_backgrounds;

    std::vector<ssize_t> visited_node_count_on_height;
    std::vector<ssize_t> visited_key_count_on_height;

    ssize_t GetHeight(NodeForDraw* vertex) {
        if (vertex == nullptr) {
            return 0;
        }
        ssize_t result = 0;
        for (ssize_t i = 0; i < std::ssize(vertex->children); ++i) {
            result = std::max(result, GetHeight(vertex->children[i].get()));
        }
        return result + 1;
    }
    void RecalculateCountVectors() {
        auto actual_height = GetHeight(root.get());
        node_count_on_height.assign(actual_height, 0);
        key_count_on_height.assign(actual_height, 0);
        RecursiveRecalcOfVectors(root.get(), actual_height);
    }
    void RecursiveRecalcOfVectors(NodeForDraw* vertex, ssize_t height_of_vertex) {
        ++node_count_on_height[height_of_vertex];
        key_count_on_height[height_of_vertex] += vertex->keys.size();
        for (ssize_t i = 0; i < std::ssize(vertex->children); ++i) {
            RecursiveRecalcOfVectors(vertex->children[i].get(), height_of_vertex + 1);
        }
    }

    void DrawTree(QGraphicsScene* scene) {
        visited_node_count_on_height.assign(node_count_on_height.size(), 0);
        visited_key_count_on_height.assign(key_count_on_height.size(), 0);
        scene->clear();
        RecursiveDrawTree(root.get(), scene);
    }
    void RecursiveDrawTree(NodeForDraw* vertex, QGraphicsScene* scene, ssize_t current_height = 0) {
        if (vertex == nullptr) {
            return;
        }
        for (ssize_t i = 0; i < std::ssize(vertex->keys); ++i) {
            QBrush brush(vertex->background_color);
            QPen pen(QColorConstants::Black);
            auto rectangle =
                scene->addRect(visited_node_count_on_height[current_height] * kVerticalMargin +
                                   visited_key_count_on_height[current_height] * kCellWidth,
                               current_height * (kCellHeight + kVerticalMargin), kCellWidth, kCellHeight, pen, brush);
            scene->addText(QString::number(vertex->keys[i]))->setPos(rectangle->pos());
        }
    }
};

View::View() : port_([]() {}, [this](TreeActionsBatch changes) { this->HandleNotification(changes); }, []() {}) {}

void View::HandleNotification(TreeActionsBatch changes) {
    for (const auto& change : changes) {
        switch (change.action) {
        case ENodeAction::StartQuery:
            assert(changes.size() == 1 && "Incorrect batch with StartQuery action");
            // Note: this case is significant for async animation. It doesn't matter while animation happens in
            // signal-handler (being a long-time operation), so we simply check a correctness of an operation.
            assert(storage_.empty());
            return;
        case ENodeAction::EndQuery:
            assert(changes.size() == 1 && "Incorrect batch with EndQuery action");
            AnimateQueries();
            return;
        case ENodeAction::Create:
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
