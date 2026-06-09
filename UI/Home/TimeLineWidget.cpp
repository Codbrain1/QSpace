#include "TimeLineWidget.h"
#include "ui_TimeLineWidget.h"

namespace QSpace::UI {
TimeLineWidget::TimeLineWidget(Core::AppCore* app, QWidget* parent)
    : QWidget(parent), ui(new Ui::TimeLineWidget), m_app(app) {
    ui->setupUi(this);
}
void TimeLineWidget::on_toolButtonPrev_clicked() {
    // Реализация кнопки предыдущего кадра
}
void TimeLineWidget::on_toolButtonNext_clicked() {
    // Реализация кнопки следующего кадра
}
void TimeLineWidget::on_TimeSlider_valueChanged(int value) {
    // Реализация изменения значения слайдера
}
TimeLineWidget::~TimeLineWidget() {
    delete ui;
}
} // namespace QSpace::UI