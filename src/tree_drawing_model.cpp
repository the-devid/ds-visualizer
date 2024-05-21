#include "tree_drawing_model.h"

#include <QColor>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QLine>
#include <QPen>
#include <QRect>

#include <cassert>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace NVis {

class TreeDrawingModel::TreeDrawingModelImpl {
    static constexpr qreal kCellWidth = 50;
    static constexpr qreal kCellHeight = 30;
    static constexpr qreal kVerticalMargin = 50;
    static constexpr qreal kHorizontalMargin = 50;

    struct NodeForDraw {
        std::vector<Key> keys;
        std::vector<MemoryAddress> children;
        QColor background_color = QColorConstants::White;
    };

public:
    void DrawActions(const TreeActionsBatch& actions, QGraphicsScene* scene) {
        for (const auto& action : actions) {
            switch (action.action_type) {

            case ENodeAction::StartQuery:
                [[fallthrough]];
            case ENodeAction::EndQuery:
                break;
            case ENodeAction::Create:
                assert(!address_to_node_.contains(action.node_address) && "Creating already existed node");
                assert(action.data.has_value() && "No data when creating new node");
                address_to_node_[action.node_address] = NodeForDraw{
                    .keys = action.data->keys,
                    .children = action.data->children,
                    .background_color = QColorConstants::Green,
                };
                break;
            case ENodeAction::Delete:
                assert(address_to_node_.contains(action.node_address) && "Deleting non-existing node");
                address_to_node_.erase(action.node_address);
                break;
            case ENodeAction::Change:
                assert(address_to_node_.contains(action.node_address) && "Changing non-existing node");
                assert(action.data.has_value() && "No data when changing a node");
                address_to_node_[action.node_address] = NodeForDraw{
                    .keys = action.data->keys,
                    .children = action.data->children,
                    .background_color = QColorConstants::Yellow,
                };
                break;
            case ENodeAction::MakeRoot:
                assert((action.node_address == nullptr || address_to_node_.contains(action.node_address)) &&
                       "Making a non-existing node a root");
                root_ = action.node_address;
                break;
            case ENodeAction::Visit:
                assert(address_to_node_.contains(action.node_address) && "Visiting a non-existing node");
                address_to_node_[action.node_address].background_color = QColorConstants::Cyan;
                break;
            }
        }
        DrawTree(scene);
    }

private:
    void RecalculateLeafCounters() {
        leaf_node_count_ = 0;
        leaf_key_count_ = 0;
        RecalculateLeafCountersRecursively(root_);
    }

    void RecalculateLeafCountersRecursively(MemoryAddress vertex) {
        if (vertex == nullptr) {
            return;
        }
        // Leaf is a node without children.
        if (address_to_node_[vertex].children.empty()) {
            leaf_node_count_ += 1;
            leaf_key_count_ += std::ssize(address_to_node_[vertex].keys);
        }
        for (ssize_t i = 0; i < std::ssize(address_to_node_[vertex].children); ++i) {
            RecalculateLeafCountersRecursively(address_to_node_[vertex].children[i]);
        }
    }

    void DrawTree(QGraphicsScene* scene) {
        RecalculateLeafCounters();
        visited_leaf_key_count_ = 0;
        visited_leaf_node_count_ = 0;
        visited_nodes_.clear();
        scene->clear();
        RecursiveDrawTree(root_, scene);
        CleanBackgroundRecursively(root_);
        // Garbage collecting. First we write all the nodes we don't need to store, then erase them.
        std::unordered_set<MemoryAddress> nodes_to_delete;
        for (const auto& [address, node] : address_to_node_) {
            if (!visited_nodes_.contains(address)) {
                nodes_to_delete.insert(address);
            }
        }
        for (auto deleting_address : nodes_to_delete) {
            address_to_node_.erase(deleting_address);
        }
    }

    //! Returns top-middle point of the rectangle, which bounds keys that are drawn on call.
    std::optional<QPointF> RecursiveDrawTree(MemoryAddress vertex, QGraphicsScene* scene, ssize_t current_height = 0) {
        if (vertex == nullptr) {
            // One probably should think of `if (root == nullptr)` instead of using `optional`.
            return std::nullopt;
        }
        // Maybe "left to us" is better to understand than "lefter"...
        auto lefter_leaf_node_count = visited_leaf_node_count_;
        auto lefter_leaf_key_count = visited_leaf_key_count_;
        visited_nodes_.emplace(vertex);
        std::optional<QPointF> top_left_corner;
        std::vector<QPointF> children_positions;
        children_positions.reserve(address_to_node_[vertex].children.size());
        // Important invariant of this function is that we first traverse through our children and only then draw
        // ourselves.
        for (ssize_t i = 0; i < std::ssize(address_to_node_[vertex].children); ++i) {
            auto child_position = RecursiveDrawTree(address_to_node_[vertex].children[i], scene, current_height + 1);
            assert(child_position.has_value() && "Incorrect position of rectangle when drawing");
            children_positions.emplace_back(child_position.value());
        }
        if (address_to_node_[vertex].children.empty()) {
            visited_leaf_node_count_++;
            visited_leaf_key_count_ += std::ssize(address_to_node_[vertex].keys);
        }
        qreal left_subtree_border = lefter_leaf_key_count * kCellWidth + lefter_leaf_node_count * kHorizontalMargin;
        qreal right_subtree_border =
            visited_leaf_key_count_ * kCellWidth + (visited_leaf_node_count_ - 1) * kHorizontalMargin;
        // "A middle point of the node being drawn". Try to fit it in a variable's name. And yes, we could write
        // `(l+r)/2` instead of `l+(r-l)/2`, but second option seems more precision-friendly and intuitive.
        qreal drawing_node_midpoint = left_subtree_border + (right_subtree_border - left_subtree_border) / 2.0;
        top_left_corner = QPointF(drawing_node_midpoint - address_to_node_[vertex].keys.size() * kCellWidth / 2.0,
                                  current_height * (kCellHeight + kHorizontalMargin));

        for (ssize_t i = 0; i < std::ssize(address_to_node_[vertex].keys); ++i) {
            auto position_to_draw = QPointF(top_left_corner->x() + i * kCellWidth, top_left_corner->y());
            auto rectangle_item = scene->addRect(position_to_draw.x(), position_to_draw.y(), kCellWidth, kCellHeight,
                                                 QPen(), QBrush(address_to_node_[vertex].background_color));
            auto text_item = scene->addText(QString::number(address_to_node_[vertex].keys[i]));
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

    void CleanBackgroundRecursively(MemoryAddress vertex) {
        if (vertex == nullptr) {
            return;
        }
        address_to_node_[vertex].background_color = QColorConstants::White;
        for (ssize_t i = 0; i < std::ssize(address_to_node_[vertex].children); ++i) {
            CleanBackgroundRecursively(address_to_node_[vertex].children[i]);
        }
    }

    MemoryAddress root_ = nullptr;
    //! Not only maps Model nodes' addresses to drawable nodes, but also owns them.
    std::unordered_map<MemoryAddress, NodeForDraw> address_to_node_;

    ssize_t leaf_node_count_;
    ssize_t leaf_key_count_;
    ssize_t visited_leaf_node_count_;
    ssize_t visited_leaf_key_count_;
    // This is for "garbage collection" purposes.
    std::unordered_set<MemoryAddress> visited_nodes_;
};

TreeDrawingModel::TreeDrawingModel() : impl_(std::make_unique<TreeDrawingModelImpl>()) {}

TreeDrawingModel::~TreeDrawingModel() = default;

void TreeDrawingModel::DrawActions(const TreeActionsBatch& actions) {
    impl_->DrawActions(actions, &scene_);
}

QGraphicsScene* TreeDrawingModel::GetScenePort() {
    return &scene_;
}

} // namespace NVis
