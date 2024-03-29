#pragma once

#include <vector>

namespace NVis {

using Key = int;
using MemoryAddress = void*;

class TwoThreeTree;

enum class ENodeAction;
struct NodeInfo;
struct TreeAction;
using TreeActionsBatch = std::vector<TreeAction>;

} // namespace NVis
