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
    void currentTimeStampValueChanged(int valueTimestamp, bool isPreview);

  private slots:
    void handleToolButtonPrev_clicked();
    void handleToolButtonNext_clicked();
    void handleTimeSliderChangeValue(int value);

    void handleSnapshotsListChangeSize(int size);

  private:
    Ui::TimeLineWidget* ui;
    Core::AppCore*      m_app;
    void                setTextTimeLabelTextInternal();
};
} // namespace QSpace::UI