#pragma once

#include "controller.h"
#include "two_three_tree.h"
#include "view.h"
#include "window.h"

namespace NVis {

class Application {
public:
    Application();
    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;
    ~Application() = default;

    void ShowWindow();

private:
    AnimationProducer animation_producer_;
    TreeDrawingModel drawing_model_;
    Window window_;
    Model model_;
    Controller controller_;
};

} // namespace NVis
