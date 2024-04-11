#pragma once

#include "public.h"

#include <QGraphicsView>
#include <QGridLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QObject>
#include <QPushButton>

namespace NVis {

class Window : public QMainWindow {
    Q_OBJECT
public:
    Window(QWidget* parent = nullptr);

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

private:
    QGridLayout* layout_;
    QGraphicsView* view_;
    QLineEdit* key_edit_;
    QPushButton* insert_button_;
    QPushButton* erase_button_;
    QPushButton* search_button_;

    static constexpr int kWidth = 1280;
    static constexpr int kHeight = 720;

    friend class Application;
};

} // namespace NVis
