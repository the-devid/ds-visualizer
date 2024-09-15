#pragma once

#include "two_three_tree.h"

#include <QLineEdit>
#include <QObject>
#include <QPushButton>

namespace NVis {

using Model = TwoThreeTree;

class Controller : public QObject {
    Q_OBJECT
public:
    Controller(Model* model, QLineEdit* key_edit);

public slots:
    void OnInsertButtonClick();
    void OnEraseButtonClick();
    void OnSearchButtonClick();

private:
    std::optional<Key> TryGetKeyFromEdit() const;

    Model* model_ = nullptr;
    QLineEdit* key_edit_ = nullptr;
};

} // namespace NVis
