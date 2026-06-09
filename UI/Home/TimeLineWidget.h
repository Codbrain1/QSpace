#pragma once
#include "Core/AppCore/AppCore.h"
#include <QWidget>
QT_BEGIN_NAMESPACE
namespace Ui {
class TimeLineWidget;
}
QT_END_NAMESPACE

namespace QSpace::UI {
class TimeLineWidget : public QWidget {
    Q_OBJECT
  public:
    explicit TimeLineWidget(Core::AppCore* app, QWidget* parent = nullptr);
    ~TimeLineWidget();

  private slots:
    void on_toolButtonPrev_clicked();
    void on_toolButtonNext_clicked();
    void on_TimeSlider_valueChanged(int value);

  private:
    Ui::TimeLineWidget* ui;
    Core::AppCore*      m_app;
};
} // namespace QSpace::UI