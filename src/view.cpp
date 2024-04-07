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
#include <QTimer>

#include <cassert>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace NVis {

struct View::DrawingInfo {
    static constexpr int kCellWidth = 50;
    static constexpr int kCellHeight = 30;
    static constexpr int kVerticalMargin = 50;
    static constexpr int kHorizontalMargin = 50;

    struct NodeForDraw {
        std::vector<Key> keys;
        std::vector<MemoryAddress> children;
        QColor background_color = QColorConstants::White;
    };
    MemoryAddress root = nullptr;
    //! Actually stores node drawing information.
    std::unordered_map<MemoryAddress, NodeForDraw> address_to_node;
    std::vector<ssize_t> node_count_on_height;
    std::vector<ssize_t> key_count_on_height;

    std::vector<ssize_t> visited_node_count_on_height;
    std::vector<ssize_t> visited_key_count_on_height;
    std::unordered_set<MemoryAddress> visited_nodes;

    ssize_t GetHeight(MemoryAddress vertex) {
        if (vertex == nullptr) {
            return 0;
        }
        ssize_t result = 0;
        for (ssize_t i = 0; i < std::ssize(address_to_node[vertex].children); ++i) {
            result = std::max(result, GetHeight(address_to_node[vertex].children[i]));
        }
        return result + 1;
    }
    void RecalculateCountVectors() {
        auto actual_height = GetHeight(root);
        node_count_on_height.assign(actual_height, 0);
        key_count_on_height.assign(actual_height, 0);
        RecursiveRecalcOfVectors(root, actual_height);
    }
    void RecursiveRecalcOfVectors(MemoryAddress vertex, ssize_t height_of_vertex) {
        ++node_count_on_height[height_of_vertex];
        key_count_on_height[height_of_vertex] += address_to_node[vertex].keys.size();
        for (ssize_t i = 0; i < std::ssize(address_to_node[vertex].children); ++i) {
            RecursiveRecalcOfVectors(address_to_node[vertex].children[i], height_of_vertex + 1);
        }
    }

    void DrawTree(QGraphicsScene* scene) {
        RecalculateCountVectors();
        visited_node_count_on_height.assign(node_count_on_height.size(), 0);
        visited_key_count_on_height.assign(key_count_on_height.size(), 0);
        visited_nodes.clear();
        scene->clear();
        RecursiveDrawTree(root, scene);
        CleanUpRecursively(root);
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
    //! Returns top-left corner of rectangle being drawn on call.
    std::optional<QPointF> RecursiveDrawTree(MemoryAddress vertex, QGraphicsScene* scene, ssize_t current_height = 0) {
        if (vertex == nullptr) {
            return std::nullopt;
        }
        visited_nodes.emplace(vertex);
        std::optional<QPointF> top_left_corner;
        std::vector<QPointF> bottom_left_corners;
        for (ssize_t i = 0; i < std::ssize(address_to_node[vertex].keys); ++i) {
            auto position_to_draw = QPoint(visited_node_count_on_height[current_height] * kVerticalMargin +
                                               visited_key_count_on_height[current_height] * kCellWidth,
                                           current_height * (kCellHeight + kVerticalMargin))
                                        .toPointF();
            if (!top_left_corner.has_value()) {
                top_left_corner = position_to_draw;
            }
            bottom_left_corners.emplace_back(top_left_corner->x(), top_left_corner->y() + kCellHeight);
            QBrush brush(address_to_node[vertex].background_color);
            QPen pen(QColorConstants::Black);
            scene->addRect(position_to_draw.x(), position_to_draw.y(), kCellWidth, kCellHeight, pen, brush);
            scene->addText(QString::number(address_to_node[vertex].keys[i]))->setPos(position_to_draw);
            ++visited_key_count_on_height[current_height];
        }
        ++visited_node_count_on_height[current_height];
        for (ssize_t i = 0; i < std::ssize(address_to_node[vertex].children); ++i) {
            auto end_of_arrow = RecursiveDrawTree(address_to_node[vertex].children[i], scene, current_height + 1);
            assert(end_of_arrow.has_value() && "Incorrect position of rectangle when drawing");
            scene->addLine(QLineF(bottom_left_corners[i], end_of_arrow.value()));
        }
        return top_left_corner;
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
                assert(drawing_info_ptr_->address_to_node.contains(action.node_address) &&
                       "Making a non-existing node a root");
                drawing_info_ptr_->root = action.node_address;
            case ENodeAction::Visit:
                assert(drawing_info_ptr_->address_to_node.contains(action.node_address) &&
                       "Visiting a non-existing node");
                drawing_info_ptr_->address_to_node[action.node_address].background_color = QColorConstants::Cyan;
            }
        }
        drawing_info_ptr_->DrawTree(&scene_);
        QEventLoop loop;
        QTimer t;
        t.setSingleShot(true);
        QTimer::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
        t.start(100);
        loop.exec();
    }
    storage_.clear();
}

} // namespace NVis
