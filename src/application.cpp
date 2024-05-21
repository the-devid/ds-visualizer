#include "application.h"

#include "view.h"

namespace NVis {

Application& Application::Instance() {
    static Application app;
    return app;
}

Application::Application()
    : animation_producer_(&drawing_model_),
      drawing_model_(),
      window_(),
      model_(),
      controller_(&model_, window_.GetKeyEdit()) {

    QObject::connect(window_.GetInsertButton(), &QPushButton::clicked, &controller_, &Controller::OnInsertButtonClick);
    QObject::connect(window_.GetEraseButton(), &QPushButton::clicked, &controller_, &Controller::OnEraseButtonClick);
    QObject::connect(window_.GetSearchButton(), &QPushButton::clicked, &controller_, &Controller::OnSearchButtonClick);

    model_.SubscribeObserver(animation_producer_.GetTreeActionsPort());
    window_.SubscribeViewWidgetTo(drawing_model_.GetScenePort());
    window_.show();
}

} // namespace NVis
