#pragma once

#include <algorithm>
#include <memory>
#include <vector>

namespace NVis {

template <class T>
class TwoThreeTree {
public:
    //! Searches for the key `x` in 2-3 tree and returns erther it was found or not.
    bool Contains(const T& x) const {
        auto node_was_found = SearchByLowerBound(x);
        if (node_was_found == nullptr) {
            return false;
        }

        for (const auto& key : node_was_found->keys) {
            if (key == x) {
                return true;
            }
        }
        return false;
    }

    //! Inserts the key `x` in 2-3 tree or do nothing if it already was there. Returns `true` if new key was added or
    //! `false` if it already was there.
    bool Insert(const T& x) {
        if (root_ == nullptr) {
            root_ = std::make_unique<Node>(Node{.keys = {x}, .children = {}, .parent = nullptr});
            return true;
        }
        auto node_was_found = SearchByLowerBound(x);
        assert(node_was_found->children.empty() && "Descent in 2-3 tree returned not a leaf");

        if (std::find(node_was_found->keys.begin(), node_was_found->keys.end(), x) != node_was_found->keys.end()) {
            return false;
        }
        node_was_found->keys.emplace(std::find_if(node_was_found->keys.begin(), node_was_found->keys.end(),
                                                  [&x](const T& key) { return x < key; }),
                                     x);
        UpdateKeys(node_was_found);
        SplitNode(node_was_found);
        return true;
    }

    //! Erases the key `x` from 2-3 tree if it was there or do nothing otherwise. Returns `true` if key was deleted or
    //! `false` otherwise.
    bool Erase(const T& x) {
        auto node_was_found = SearchByLowerBound(x);
        if (node_was_found == nullptr) {
            return false;
        }
        assert(node_was_found->children.empty() && "Descent in 2-3 tree returned not a leaf");
        auto vertex = node_was_found;
        ssize_t erasing_ind = std::find(vertex->keys.begin(), vertex->keys.end(), x) - vertex->keys.begin();

        if (erasing_ind == vertex->keys.size()) {
            return false;
        }
        while (erasing_ind != vertex->keys.size()) {
            vertex->keys.erase(vertex->keys.begin() + erasing_ind);
            if (vertex->children.empty()) {
                // Processing a leaf. It has no children to delete, but erasing a key can lead to necessity of updating
                // keys.
                UpdateKeys(vertex);
            } else {
                // Processing an internal vertex. No need to update keys, but need to also erase one of children.
                vertex->children.erase(vertex->children.begin() + erasing_ind);
            }
            if (vertex->keys.size() > 1) {
                break;
            }
            auto parent = vertex->parent;
            if (parent == nullptr) {
                assert(root_.get() == vertex && "Non root vertex has no parent");
                if (!vertex->children.empty()) {
                    root_ = std::move(root_->children[0]);
                    root_->parent = nullptr;
                }
                break;
            }
            ssize_t in_parent_ind =
                std::find_if(parent->children.begin(), parent->children.end(),
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
                SplitNode(sibling);
                break;
            } else {
                erasing_ind = in_parent_ind;
                vertex = parent;
            }
        }
        return true;
    }

private:
    struct Node {
        std::vector<T> keys;
        std::vector<std::unique_ptr<Node>> children;
        Node* parent;
    };
    std::unique_ptr<Node> root_;

    //! Searches such a leaf in the tree that contains the first value greater or equal to `x`. If there's no such one,
    //! returns `nullptr`.
    Node* SearchByLowerBound(const T& x) const {
        auto vertex = root_.get();
        if (vertex == nullptr) {
            return nullptr;
        }
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
        }
        return vertex;
    }

    //! Updates keys in parents of `vertex` if needed by pulling up information from children.
    void UpdateKeys(Node* vertex) {
        assert(vertex != nullptr && "Trying to update keys of a nullptr in 2-3-tree");
        while (vertex->parent != nullptr) {

            vertex = vertex->parent;
            vertex->keys.resize(vertex->children.size());
            for (ssize_t key_index = 0; key_index < std::ssize(vertex->keys); ++key_index) {
                vertex->keys[key_index] = vertex->children[key_index]->keys.back();
            }
        }
    }

    //! Splits a node in two nodes if it has more than 4 children (or keys), and all its parents that need it after
    //! splitting the initial node.
    void SplitNode(Node* vertex) {
        assert(vertex != nullptr && "Trying to split nullptr in 2-3-tree");
        while (vertex->keys.size() > 3) {
            assert(vertex->keys.size() == 4 && "Some node in 2-3-tree has more than 4 keys at split "
                                               "stage");
            auto first_node = std::make_unique<Node>(
                Node{.keys = {vertex->keys[0], vertex->keys[1]}, .children = {}, .parent = nullptr});

            auto second_node = std::make_unique<Node>(
                Node{.keys = {vertex->keys[2], vertex->keys[3]}, .children = {}, .parent = nullptr});

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

                vertex = parent;
            }
        }
    }

    void CheckInvariants(Node* vertex) {
        if (vertex == nullptr) {
            return;
        }
        for (ssize_t child_ind = 0; child_ind < std::ssize(vertex->children); ++child_ind) {
            assert(vertex->children[child_ind] != nullptr && "Incorrect child in 2-3-tree");
            assert(vertex->children[child_ind]->parent == vertex &&
                   "Child's `parent` field points to different vertex");
        }
        for (ssize_t child_ind = 0; child_ind < std::ssize(vertex->children); ++child_ind) {
            CheckInvariants(vertex->children[child_ind].get());
        }
    }
};

} // namespace NVis
