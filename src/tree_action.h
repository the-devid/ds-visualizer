#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace NVis {

enum class ENodeAction {
    Visit,
    Create,
    Delete,
    Change,
    MakeRoot,
    StartQuery,
    EndQuery,
};

using Key = int;
using MemoryAddress = void*;

struct NodeInfo {
    std::vector<Key> keys;
    std::vector<MemoryAddress> children;
};

struct TreeAction {
    MemoryAddress node_address = nullptr;
    ENodeAction action_type;
    // For `Visit`, `Delete` and `MakeRoot` node actions we don't need any info other than node's address. For `Create`
    // and `Change` node actions we want to transfer node's new state.
    std::optional<NodeInfo> data = std::nullopt;
};

using TreeActionsBatch = std::vector<TreeAction>;

} // namespace NVis
