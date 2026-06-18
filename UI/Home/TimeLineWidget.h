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

  signals:
    void currentTimeStampValueChanged(int valueTimestamp);
  private slots:
    void handleToolButtonPrev_clicked();
    void handleToolButtonNext_clicked();
    void handleTimeSlider_valueChanged(int value);
    void handleSnapshotsSizeChange(int size);

  private:
    Ui::TimeLineWidget* ui;
    Core::AppCore*      m_app;
    QTimer              m_updateTimer;
    int                 m_pendingSliderValue = -1;
    void                setTimeLabelTextInternal();
};
} // namespace QSpace::UI