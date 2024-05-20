#include "application.h"

#include "src/view.h"
#include "src/window.h"

namespace NVis {

Application& Application::Instance() {
    static Application app;
    return app;
}

Application::Application() : view_(), window_(), model_(), controller_(&model_, window_.GetKeyEdit()) {

    QObject::connect(window_.GetInsertButton(), &QPushButton::clicked, &controller_, &Controller::OnInsertButtonClick);
    QObject::connect(window_.GetEraseButton(), &QPushButton::clicked, &controller_, &Controller::OnEraseButtonClick);
    QObject::connect(window_.GetSearchButton(), &QPushButton::clicked, &controller_, &Controller::OnSearchButtonClick);

    model_.SubscribeObserver(view_.GetTreeActionsPort());
    window_.SubscribeViewWidgetTo(view_.GetGraphicsModelPort());
    window_.show();
}

} // namespace NVis
