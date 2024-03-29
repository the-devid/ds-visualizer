#pragma once

#include "observer.h"
#include "public.h"
#include "tree_action.h"

#include <memory>
#include <vector>

namespace NVis {

class TwoThreeTree {
public:
    //! Searches for the key `x` in 2-3 tree and returns erther it was found or not.
    bool Contains(const Key& x) const;

    //! Inserts the key `x` in 2-3 tree or do nothing if it already was there. Returns `true` if new key was added or
    //! `false` if it already was there.
    bool Insert(const Key& x);

    //! Erases the key `x` from 2-3 tree if it was there or do nothing otherwise. Returns `true` if key was deleted or
    //! `false` otherwise.
    bool Erase(const Key& x);

private:
    struct Node {
        std::vector<Key> keys;
        std::vector<std::unique_ptr<Node>> children;
        Node* parent;
    };
    std::unique_ptr<Node> root_;
    // Yes, this field is mutable so it's possible to, for example, some observer to unsubscribe on notify when
    // processing query like `Conatins`.
    mutable Observable<TreeActionsBatch> port_;

    //! Searches such a leaf in the tree that contains the first value greater or equal to `x`. If there's no such
    //! one, returns the rightmost leaf.
    Node* SearchByLowerBound(const Key& x) const;

    //! Updates keys in parents of `vertex` if needed by pulling up information from children.
    void UpdateKeys(Node* vertex);

    //! Splits a node in two nodes if it has more than 4 children (or keys), and all its parents that need it after
    //! splitting the initial node.
    void SplitNode(Node* vertex);

    //! Checks invariants of a tree and return `true` if tree is valid, `false` otherwise. Suitable for `assert`s.
    bool IsValid(Node* vertex);

    static NodeInfo ProduceNodeInfo(const Node& martyr);
};

} // namespace NVis
