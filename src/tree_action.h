#pragma once

#include "public.h"

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

struct NodeInfo {
    std::vector<Key> keys;
    std::vector<MemoryAddress> children;
};

struct TreeAction {
    MemoryAddress node_address;
    ENodeAction action;
    // For `Visit`, `Delete` and `MakeRoot` node actions we don't need any info other than node's address. For `Create`
    // and `Change` node actions we want to transfer node's new state.
    std::optional<NodeInfo> data;
};

} // namespace NVis
