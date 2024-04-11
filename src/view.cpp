#include "view.h"

#include "observer.h"
#include "public.h"
#include "tree_action.h"

#include <QColor>
#include <QEventLoop>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QLine>
#include <QPen>
#include <QRect>
#include <QThread>
#include <QTimer>

#include <cassert>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace NVis {

struct View::DrawingInfo {
    static constexpr qreal kCellWidth = 50;
    static constexpr qreal kCellHeight = 30;
    static constexpr qreal kVerticalMargin = 50;
    static constexpr qreal kHorizontalMargin = 50;

    struct NodeForDraw {
        std::vector<Key> keys;
        std::vector<MemoryAddress> children;
        QColor background_color = QColorConstants::White;
    };
    MemoryAddress root = nullptr;
    //! Not only maps Model nodes' addresses to drawable nodes, but also owns them.
    std::unordered_map<MemoryAddress, NodeForDraw> address_to_node;

    // std::vector<ssize_t> visited_key_count_on_height;
    ssize_t leaf_node_count;
    ssize_t leaf_key_count;
    ssize_t visited_leaf_node_count;
    ssize_t visited_leaf_key_count;
    // This is for "garbage collection" purposes.
    std::unordered_set<MemoryAddress> visited_nodes;

    void RecalculateLeafCounters() {
        leaf_node_count = 0;
        leaf_key_count = 0;
        RecalculateLeafCountersRecursively(root);
    }
    void RecalculateLeafCountersRecursively(MemoryAddress vertex) {
        if (vertex == nullptr) {
            return;
        }
        // Leaf is a node without children.
        if (address_to_node[vertex].children.empty()) {
            leaf_node_count += 1;
            leaf_key_count += std::ssize(address_to_node[vertex].keys);
        }
        for (ssize_t i = 0; i < std::ssize(address_to_node[vertex].children); ++i) {
            RecalculateLeafCountersRecursively(address_to_node[vertex].children[i]);
        }
    }
    void DrawTree(QGraphicsScene* scene) {
        RecalculateLeafCounters();
        visited_leaf_key_count = 0;
        visited_leaf_node_count = 0;
        visited_nodes.clear();
        scene->clear();
        RecursiveDrawTree(root, scene);
        // Cleaning backround colors for future.
        CleanUpRecursively(root);
        // Garbage collecting. First we write all the nodes we don't need to store, then erase them.
        std::unordered_set<MemoryAddress> nodes_to_delete;
        for (const auto& [address, node] : address_to_node) {
            if (!visited_nodes.contains(address)) {
                nodes_to_delete.insert(address);
            }
        }
        for (auto deleting_address : nodes_to_delete) {
            address_to_node.erase(deleting_address);
        }
    }
    //! Returns top-middle point of the rectangle bounding keys being drawn on call.
    std::optional<QPointF> RecursiveDrawTree(MemoryAddress vertex, QGraphicsScene* scene, ssize_t current_height = 0) {
        if (vertex == nullptr) {
            // One probably should think of `if (root == nullptr)` instead of using `optional`.
            return std::nullopt;
        }
        // Maybe "left to us" is better to understand than "lefter"...
        auto lefter_leaf_node_count = visited_leaf_node_count;
        auto lefter_leaf_key_count = visited_leaf_key_count;
        visited_nodes.emplace(vertex);
        std::optional<QPointF> top_left_corner;
        std::vector<QPointF> children_positions;
        children_positions.reserve(address_to_node[vertex].children.size());
        // Important invariant of this function is that we first traverse through our children and only then draw
        // ourselves.
        for (ssize_t i = 0; i < std::ssize(address_to_node[vertex].children); ++i) {
            auto child_position = RecursiveDrawTree(address_to_node[vertex].children[i], scene, current_height + 1);
            assert(child_position.has_value() && "Incorrect position of rectangle when drawing");
            children_positions.emplace_back(child_position.value());
        }
        if (address_to_node[vertex].children.empty()) {
            visited_leaf_node_count++;
            visited_leaf_key_count += std::ssize(address_to_node[vertex].keys);
        }
        qreal left_subtree_border = lefter_leaf_key_count * kCellWidth + lefter_leaf_node_count * kHorizontalMargin;
        qreal right_subtree_border =
            visited_leaf_key_count * kCellWidth + (visited_leaf_node_count - 1) * kHorizontalMargin;
        // "A middle point of the node being drawn". Yeah, try to fit it in a variable's name. I'd prefer to write code
        // in cyrillic at moments like that... And yes, I could write `(l+r)/2` instead of `l+(r-l)/2`, but second
        // option seems more precision-friendly and intuitive.
        qreal drawing_node_midpoint = left_subtree_border + (right_subtree_border - left_subtree_border) / 2.0;
        top_left_corner = QPointF(drawing_node_midpoint - address_to_node[vertex].keys.size() * kCellWidth / 2.0,
                                  current_height * (kCellHeight + kHorizontalMargin));

        for (ssize_t i = 0; i < std::ssize(address_to_node[vertex].keys); ++i) {
            auto position_to_draw = QPointF(top_left_corner->x() + i * kCellWidth, top_left_corner->y());
            auto rectangle_item = scene->addRect(position_to_draw.x(), position_to_draw.y(), kCellWidth, kCellHeight,
                                                 QPen(), QBrush(address_to_node[vertex].background_color));
            auto text_item = scene->addText(QString::number(address_to_node[vertex].keys[i]));
            // Positioning in the center of Cell.
            text_item->setPos(rectangle_item->mapToScene(rectangle_item->boundingRect().center()) +
                              (text_item->boundingRect().topLeft() - text_item->boundingRect().center()));
            // Scale text from the center.
            text_item->setTransformOriginPoint(text_item->boundingRect().center());
            // TODO: maybe common scale factor for all keys would look better.
            auto text_scale_factor =
                std::min(rectangle_item->boundingRect().width() / text_item->boundingRect().width(),
                         rectangle_item->boundingRect().height() / text_item->boundingRect().height());
            text_item->setScale(text_scale_factor);

            if (!children_positions.empty()) {
                scene->addLine(
                    QLineF(QPointF(position_to_draw.x() + kCellWidth / 2.0, position_to_draw.y() + kCellHeight),
                           children_positions[i]));
            }
        }
        return QPointF(drawing_node_midpoint, top_left_corner->y());
    }
    void CleanUpRecursively(MemoryAddress vertex) {
        if (vertex == nullptr) {
            return;
        }
        address_to_node[vertex].background_color = QColorConstants::White;
        for (ssize_t i = 0; i < std::ssize(address_to_node[vertex].children); ++i) {
            CleanUpRecursively(address_to_node[vertex].children[i]);
        }
    }
};

View::View()
    : port_([]() {}, [this](TreeActionsBatch changes) { this->HandleNotification(changes); }, []() {}),
      drawing_info_ptr_(std::make_unique<DrawingInfo>()) {}
View::~View() {}

void View::HandleNotification(TreeActionsBatch actions) {
    for (const auto& action : actions) {
        if (action.action_type == ENodeAction::StartQuery) {
            assert(actions.size() == 1 && "Incorrect batch with StartQuery action");
            // Note: this case is significant for async animation. It doesn't matter while animation happens in
            // signal-handler (being a long-time operation), so we simply check a correctness of an operation.
            assert(storage_.empty());
            return;
        }
        if (action.action_type == ENodeAction::EndQuery) {
            assert(actions.size() == 1 && "Incorrect batch with EndQuery action");
            AnimateQueries();
            return;
        }
    }
    storage_.emplace_back(actions);
}

Observer<TreeActionsBatch>* View::GetPort() {
    return &port_;
}

namespace {
void DoNonBlockingDelay(int delay_msec) {
    QEventLoop loop;
    QTimer t;
    t.setSingleShot(true);
    QTimer::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
    t.start(delay_msec);
    loop.exec();
}
} // namespace

void View::AnimateQueries() {
    for (const auto& actions_batch : storage_) {
        for (const auto& action : actions_batch) {
            switch (action.action_type) {

            case ENodeAction::StartQuery:
                [[fallthrough]];
            case ENodeAction::EndQuery:
                assert(false && "Incorrect action type in animation procedure");
                return;

            case ENodeAction::Create:
                assert(!drawing_info_ptr_->address_to_node.contains(action.node_address) &&
                       "Creating already existed node");
                assert(action.data.has_value() && "No data when creating new node");
                drawing_info_ptr_->address_to_node[action.node_address] = DrawingInfo::NodeForDraw{
                    .keys = action.data->keys,
                    .children = action.data->children,
                    .background_color = QColorConstants::Green,
                };
                break;
            case ENodeAction::Delete:
                assert(drawing_info_ptr_->address_to_node.contains(action.node_address) &&
                       "Deleting non-existing node");
                drawing_info_ptr_->address_to_node.erase(action.node_address);
                break;
            case ENodeAction::Change:
                assert(drawing_info_ptr_->address_to_node.contains(action.node_address) &&
                       "Changing non-existing node");
                assert(action.data.has_value() && "No data when changing a node");
                drawing_info_ptr_->address_to_node[action.node_address] = DrawingInfo::NodeForDraw{
                    .keys = action.data->keys,
                    .children = action.data->children,
                    .background_color = QColorConstants::Yellow,
                };
                break;
            case ENodeAction::MakeRoot:
                assert((action.node_address == nullptr ||
                        drawing_info_ptr_->address_to_node.contains(action.node_address)) &&
                       "Making a non-existing node a root");
                drawing_info_ptr_->root = action.node_address;
                break;
            case ENodeAction::Visit:
                assert(drawing_info_ptr_->address_to_node.contains(action.node_address) &&
                       "Visiting a non-existing node");
                drawing_info_ptr_->address_to_node[action.node_address].background_color = QColorConstants::Cyan;
                break;
            }
        }
        drawing_info_ptr_->DrawTree(&scene_);
        // TODO: very dangerous part here wants to be rewritten, because we depend on the hope that all `scene_`
        // changes will be drawn during this delay.
        DoNonBlockingDelay(kDelayBetweenFrames);
    }
    storage_.clear();
}

} // namespace NVis
