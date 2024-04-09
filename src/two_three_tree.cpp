#include "two_three_tree.h"

#include "public.h"
#include "tree_action.h"

#include <algorithm>
#include <cassert>

namespace NVis {

bool TwoThreeTree::Contains(const Key& x) const {
    port_.Notify({TreeAction{.node_address = nullptr, .action_type = ENodeAction::StartQuery}});
    auto node_was_found = SearchByLowerBound(x);
    if (node_was_found == nullptr) {
        port_.Notify({TreeAction{.node_address = nullptr, .action_type = ENodeAction::EndQuery}});
        return false;
    }

    for (const auto& key : node_was_found->keys) {
        if (key == x) {
            port_.Notify({TreeAction{.node_address = nullptr, .action_type = ENodeAction::EndQuery}});
            return true;
        }
    }
    port_.Notify({TreeAction{.node_address = nullptr, .action_type = ENodeAction::EndQuery}});
    return false;
}

bool TwoThreeTree::Insert(const Key& x) {
    port_.Notify({TreeAction{.node_address = nullptr, .action_type = ENodeAction::StartQuery}});
    if (root_ == nullptr) {
        root_ = std::make_unique<Node>(Node{.keys = {x}, .children = {}, .parent = nullptr});
        port_.Notify({TreeAction{.node_address = root_.get(),
                                 .action_type = ENodeAction::Create,
                                 .data = ProduceNodeInfo(*root_)},
                      TreeAction{.node_address = root_.get(), .action_type = ENodeAction::MakeRoot}});
        assert(IsValid(root_.get()) && "Incorrect tree after insert");
        port_.Notify({TreeAction{.node_address = nullptr, .action_type = ENodeAction::EndQuery}});
        return true;
    }
    auto node_was_found = SearchByLowerBound(x);
    assert(node_was_found->children.empty() && "Descent in 2-3 tree returned not a leaf");

    if (std::find(node_was_found->keys.begin(), node_was_found->keys.end(), x) != node_was_found->keys.end()) {
        assert(IsValid(root_.get()) && "Incorrect tree after insert");
        port_.Notify({TreeAction{.node_address = nullptr, .action_type = ENodeAction::EndQuery}});
        return false;
    }
    node_was_found->keys.emplace(std::find_if(node_was_found->keys.begin(), node_was_found->keys.end(),
                                              [&x](const Key& key) { return x < key; }),
                                 x);
    port_.Notify({TreeAction{
        .node_address = node_was_found, .action_type = ENodeAction::Change, .data = ProduceNodeInfo(*node_was_found)}});
    UpdateKeys(node_was_found);
    SplitNode(node_was_found);
    assert(IsValid(root_.get()) && "Incorrect tree after insert");

    port_.Notify({TreeAction{.node_address = nullptr, .action_type = ENodeAction::EndQuery}});
    return true;
}

bool TwoThreeTree::Erase(const Key& x) {
    port_.Notify({TreeAction{.node_address = nullptr, .action_type = ENodeAction::StartQuery}});
    auto node_was_found = SearchByLowerBound(x);
    if (node_was_found == nullptr) {
        port_.Notify({TreeAction{.node_address = nullptr, .action_type = ENodeAction::EndQuery}});
        return false;
    }
    assert(node_was_found->children.empty() && "Descent in 2-3 tree returned not a leaf");
    auto vertex = node_was_found;
    ssize_t erasing_ind = std::find(vertex->keys.begin(), vertex->keys.end(), x) - vertex->keys.begin();

    if (erasing_ind == vertex->keys.size()) {
        assert(IsValid(root_.get()) && "Incorrect tree after erase");
        port_.Notify({TreeAction{.node_address = nullptr, .action_type = ENodeAction::EndQuery}});
        return false;
    }
    // TODO: make more relevant condition for `while`.
    while (erasing_ind != vertex->keys.size()) {
        vertex->keys.erase(vertex->keys.begin() + erasing_ind);
        if (vertex->children.empty()) {
            // Processing a leaf. It has no children to delete, but erasing a key can lead to necessity of updating
            // keys.
            port_.Notify({TreeAction{
                .node_address = vertex, .action_type = ENodeAction::Change, .data = ProduceNodeInfo(*vertex)}});
            UpdateKeys(vertex);
        } else {
            // Processing an internal vertex. No need to update keys, but need to also erase one of children.
            auto erasing_address = vertex->children[erasing_ind].get();
            vertex->children.erase(vertex->children.begin() + erasing_ind);
            port_.Notify({TreeAction{.node_address = erasing_address, .action_type = ENodeAction::Delete},
                          TreeAction{.node_address = vertex,
                                     .action_type = ENodeAction::Change,
                                     .data = ProduceNodeInfo(*vertex)}});
        }
        if (vertex->keys.size() > 1) {
            break;
        }
        auto parent = vertex->parent;
        if (parent == nullptr) {
            assert(root_.get() == vertex && "Non root vertex has no parent");
            if (!vertex->children.empty()) {
                auto old_root = root_.get();
                root_ = std::move(root_->children[0]);
                root_->parent = nullptr;
                port_.Notify({TreeAction{.node_address = old_root, .action_type = ENodeAction::Delete},
                              TreeAction{.node_address = root_.get(), .action_type = ENodeAction::MakeRoot}});
            }
            break;
        }
        ssize_t in_parent_ind = std::find_if(parent->children.begin(), parent->children.end(),
                                             [vertex](const auto& child_ptr) { return child_ptr.get() == vertex; }) -
                                parent->children.begin();

        assert(in_parent_ind != parent->children.size() && "Haven't found vertex in children array of its parent");
        Node* sibling;
        if (in_parent_ind > 0) {
            // Merging to left sibling
            sibling = parent->children[in_parent_ind - 1].get();
            sibling->keys.emplace_back(vertex->keys[0]);
            parent->keys[in_parent_ind - 1] = sibling->keys.back();
            if (!vertex->children.empty()) {
                sibling->children.emplace_back(std::move(vertex->children[0]));
                sibling->children.back()->parent = sibling;
            }
        } else {
            // Merging to right sibling
            sibling = parent->children[in_parent_ind + 1].get();
            sibling->keys.emplace(sibling->keys.begin(), vertex->keys[0]);
            if (!vertex->children.empty()) {
                sibling->children.emplace(sibling->children.begin(), std::move(vertex->children[0]));
                sibling->children[0]->parent = sibling;
            }
        }
        if (sibling->keys.size() == 4) {
            parent->keys.erase(parent->keys.begin() + in_parent_ind);
            parent->children.erase(parent->children.begin() + in_parent_ind);
            port_.Notify(
                {TreeAction{
                     .node_address = sibling, .action_type = ENodeAction::Change, .data = ProduceNodeInfo(*sibling)},
                 TreeAction{
                     .node_address = parent, .action_type = ENodeAction::Change, .data = ProduceNodeInfo(*parent)}});
            SplitNode(sibling);
            break;
        } else {
            port_.Notify(
                {TreeAction{
                     .node_address = sibling, .action_type = ENodeAction::Change, .data = ProduceNodeInfo(*sibling)},
                 TreeAction{
                     .node_address = parent, .action_type = ENodeAction::Change, .data = ProduceNodeInfo(*parent)}});
            erasing_ind = in_parent_ind;
            vertex = parent;
        }
    }
    assert(IsValid(root_.get()) && "Incorrect tree after erase");
    port_.Notify({TreeAction{.node_address = nullptr, .action_type = ENodeAction::EndQuery}});
    return true;
}

TwoThreeTree::Node* TwoThreeTree::SearchByLowerBound(const Key& x) const {
    auto vertex = root_.get();
    if (vertex == nullptr) {
        return nullptr;
    }
    port_.Notify({TreeAction{.node_address = vertex, .action_type = ENodeAction::Visit}});
    while (!vertex->children.empty()) {
        bool found_child_to_go = false;

        for (ssize_t child_index = 0; child_index < std::ssize(vertex->keys); ++child_index) {
            if (x <= vertex->keys[child_index]) {
                found_child_to_go = true;
                vertex = vertex->children[child_index].get();
                break;
            }
        }
        if (!found_child_to_go) {
            vertex = vertex->children.back().get();
        }
        port_.Notify({TreeAction{.node_address = vertex, .action_type = ENodeAction::Visit}});
    }
    return vertex;
}

void TwoThreeTree::UpdateKeys(Node* vertex) {
    assert(vertex != nullptr && "Trying to update keys of a nullptr in 2-3-tree");
    while (vertex->parent != nullptr) {

        vertex = vertex->parent;
        vertex->keys.resize(vertex->children.size());
        for (ssize_t key_index = 0; key_index < std::ssize(vertex->keys); ++key_index) {
            vertex->keys[key_index] = vertex->children[key_index]->keys.back();
        }
        port_.Notify({TreeAction{
            .node_address = vertex,
            .action_type = ENodeAction::Change,
            .data = ProduceNodeInfo(*vertex),
        }});
    }
}

void TwoThreeTree::SplitNode(Node* vertex) {
    assert(vertex != nullptr && "Trying to split nullptr in 2-3-tree");
    while (vertex->keys.size() > 3) {
        assert(vertex->keys.size() == 4 && "Some node in 2-3-tree has more than 4 keys at split "
                                           "stage");
        port_.Notify({TreeAction{.node_address = vertex, .action_type = ENodeAction::Visit}});
        auto first_node =
            std::make_unique<Node>(Node{.keys = {vertex->keys[0], vertex->keys[1]}, .children = {}, .parent = nullptr});

        auto second_node =
            std::make_unique<Node>(Node{.keys = {vertex->keys[2], vertex->keys[3]}, .children = {}, .parent = nullptr});

        if (!vertex->children.empty()) {
            // Splitting not a leaf.
            assert(vertex->children.size() == 4 && "Child count doesn't match key count when splitting a "
                                                   "node in 2-3 tree");

            first_node->children.emplace_back(std::move(vertex->children[0]));
            first_node->children.emplace_back(std::move(vertex->children[1]));
            first_node->children[0]->parent = first_node.get();
            first_node->children[1]->parent = first_node.get();

            second_node->children.emplace_back(std::move(vertex->children[2]));
            second_node->children.emplace_back(std::move(vertex->children[3]));
            second_node->children[0]->parent = second_node.get();
            second_node->children[1]->parent = second_node.get();
        }
        if (vertex->parent == nullptr) {
            // Splitting root -> creating new root.
            assert(root_.get() == vertex && "Non-root node has no parent");

            // Here we use |vertex|'s keys straight to avoid UB in case of replacing order of fields in `Node` which
            // will cause performing `std::move` before access to |first_node|.
            root_ = std::make_unique<Node>(
                Node{.keys = {vertex->keys[1], vertex->keys[3]}, .children = {}, .parent = nullptr});
            root_->children.emplace_back(std::move(first_node));
            root_->children.emplace_back(std::move(second_node));
            root_->children[0]->parent = root_.get();
            root_->children[1]->parent = root_.get();
            port_.Notify({TreeAction{.node_address = vertex, .action_type = ENodeAction::Delete},
                          TreeAction{.node_address = root_->children[0].get(),
                                     .action_type = ENodeAction::Create,
                                     .data = ProduceNodeInfo(*root_->children[0])},
                          TreeAction{.node_address = root_->children[1].get(),
                                     .action_type = ENodeAction::Create,
                                     .data = ProduceNodeInfo(*root_->children[1])},
                          TreeAction{.node_address = root_.get(),
                                     .action_type = ENodeAction::Create,
                                     .data = ProduceNodeInfo(*root_)},
                          TreeAction{.node_address = root_.get(), .action_type = ENodeAction::MakeRoot}});
            return;
        } else {
            auto parent = vertex->parent;

            ssize_t inserting_index = std::ssize(parent->children);
            for (ssize_t child_index = 0; child_index < std::ssize(parent->children); ++child_index) {

                if (parent->children[child_index].get() == vertex) {
                    inserting_index = child_index;
                    break;
                }
            }
            assert(inserting_index != parent->children.size() &&
                   "Not found vertex itself in its parent's children array.");

            // We're inserting keys and children in reversed order because we don't move |inserting_index| and
            // elements of vector move to the right of place of inserting.
            parent->keys.erase(parent->keys.begin() + inserting_index);
            parent->keys.emplace(parent->keys.begin() + inserting_index, second_node->keys.back());
            parent->keys.emplace(parent->keys.begin() + inserting_index, first_node->keys.back());

            parent->children.erase(parent->children.begin() + inserting_index);
            parent->children.emplace(parent->children.begin() + inserting_index, std::move(second_node));
            parent->children.emplace(parent->children.begin() + inserting_index, std::move(first_node));
            parent->children[inserting_index]->parent = parent;
            parent->children[inserting_index + 1]->parent = parent;

            port_.Notify({TreeAction{.node_address = vertex, .action_type = ENodeAction::Delete},
                          TreeAction{.node_address = parent->children[inserting_index].get(),
                                     .action_type = ENodeAction::Create,
                                     .data = ProduceNodeInfo(*parent->children[inserting_index])},
                          TreeAction{.node_address = parent->children[inserting_index + 1].get(),
                                     .action_type = ENodeAction::Create,
                                     .data = ProduceNodeInfo(*parent->children[inserting_index + 1])},
                          TreeAction{.node_address = parent,
                                     .action_type = ENodeAction::Change,
                                     .data = ProduceNodeInfo(*parent)}});
            vertex = parent;
        }
    }
}

bool TwoThreeTree::IsValid(Node* vertex) {
    if (vertex == nullptr) {
        return true;
    }
    if (!vertex->children.empty() && vertex->children.size() != vertex->keys.size()) {
        return false; // Incorrect internal node
    }
    if (!((vertex == root_.get() && vertex->keys.size() >= 1 && vertex->keys.size() <= 3) ||
          (vertex->keys.size() >= 2 && vertex->keys.size() <= 3))) {
        return false; // Incorrect key count
    }
    for (ssize_t child_ind = 0; child_ind < std::ssize(vertex->children); ++child_ind) {
        if (vertex->children[child_ind] == nullptr) {
            return false; // Incorrect child in 2-3-tree
        }
        if (vertex->children[child_ind]->parent != vertex) {
            return false; // Child's `parent` field points to different vertex
        }
    }
    for (ssize_t child_ind = 0; child_ind < std::ssize(vertex->children); ++child_ind) {
        if (!IsValid(vertex->children[child_ind].get())) {
            return false;
        }
    }
    return true;
}

NodeInfo TwoThreeTree::ProduceNodeInfo(const Node& martyr) {
    NodeInfo result;
    result.keys = martyr.keys;
    result.children.reserve(martyr.children.size());
    for (auto& child : martyr.children) {
        result.children.emplace_back(child.get());
    }
    return result;
}

Observable<TreeActionsBatch>* TwoThreeTree::GetPort() {
    return &port_;
}

} // namespace NVis
