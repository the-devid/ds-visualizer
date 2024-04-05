#pragma once

#include "public.h"

#include <QObject>
#include <QPushButton>

namespace NVis {

class Controller : public QObject {
    Q_OBJECT
public slots:
    void OnInsertButtonClick(Key x);
    void OnEraseButtonClick(Key x);
    void OnSearchButtonClick(Key x);

private:
    QPushButton* insert_button_;
    QPushButton* erase_button_;
    QPushButton* search_button_;

    Model* model_;

    friend class Application;
};

} // namespace NVis
