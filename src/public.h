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

class Window;
class Application;

using Model = TwoThreeTree;
class View;
class Controller;

} // namespace NVis
