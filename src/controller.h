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
    Controller(Model* model, QLineEdit* key_edit, QPushButton* insert_button, QPushButton* erase_button,
               QPushButton* search_button);

public slots:
    void OnInsertButtonClick();
    void OnEraseButtonClick();
    void OnSearchButtonClick();

private:
    void HandleInsertRequest() const;
    void HandleEraseRequest() const;
    void HandleSearchRequest() const;

    std::optional<Key> TryGetKeyFromEdit() const;
    void SetButtonsState(bool enabled) const;

    Model* model_ = nullptr;
    QLineEdit* key_edit_ = nullptr;
    QPushButton* insert_button_ = nullptr;
    QPushButton* erase_button_ = nullptr;
    QPushButton* search_button_ = nullptr;
};

} // namespace NVis
