
#pragma once
#include "../AbstractView.h"

namespace QSpace::Visualize::Views {
// TODO: реализовать ветку для 2D отображения
class AbstractView2D : public AbstractView {
    Q_OBJECT
  public:
    explicit AbstractView2D(QObject* parent = nullptr);

    virtual void clearPlot() = 0;
};
} // namespace QSpace::Visualize::Views