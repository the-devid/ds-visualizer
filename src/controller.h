#pragma once

#include "public.h"

#include <QObject>
#include <QPushButton>

namespace NVis {

class Controller : public QObject {
    Q_OBJECT
public slots:
    void OnInsertButtonClick(QString input);
    void OnEraseButtonClick(QString input);
    void OnSearchButtonClick(QString input);

private:
    QPushButton* insert_button_;
    QPushButton* erase_button_;
    QPushButton* search_button_;

    Model* model_;

    void SetButtonsState(bool enabled);

    friend class Application;
};

} // namespace NVis
