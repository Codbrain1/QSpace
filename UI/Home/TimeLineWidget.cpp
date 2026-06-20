#include "TimeLineWidget.h"
#include "Common/Logger/Logger.h"
#include "Core/AppCore/DataController.h"
#include "ui_TimeLineWidget.h"
#include <qloggingcategory.h>
#include <qtoolbutton.h>

namespace QSpace::UI {

TimeLineWidget::TimeLineWidget(Core::AppCore* app, QWidget* parent)
    : QWidget(parent), ui(new Ui::TimeLineWidget), m_app(app) {
    ui->setupUi(this);

    connect(m_app->dataController(),
            &QSpace::Core::Controllers::DataController::snapshotsListSizeChanged,
            this,
            &TimeLineWidget::handleSnapshotsListChangeSize);
    connect(this,
            &TimeLineWidget::currentTimeStampValueChanged,
            m_app->dataController(),
            &QSpace::Core::Controllers::DataController::handleTimeSliderValueChanged);
    // 1. Обработка изменения значения (перетаскивание или программный ввод)
    connect(ui->TimeSlider,
            &QSlider::valueChanged,
            this,
            &TimeLineWidget::handleTimeSliderChangeValue); // TODO: транслировать сигнал qt на собственный
                                                           // сигнал класса

    connect(ui->toolButtonPrev, &QToolButton::clicked, this, &TimeLineWidget::handleToolButtonPrev_clicked);
    connect(ui->toolButtonNext, &QToolButton::clicked, this, &TimeLineWidget::handleToolButtonNext_clicked);
}

// =========== Кнопки смены кадров ===========
void TimeLineWidget::handleToolButtonPrev_clicked() {
    int currentValue = ui->TimeSlider->value();
    if (currentValue > ui->TimeSlider->minimum()) {
        ui->TimeSlider->setValue(currentValue - ui->TimeSlider->singleStep());
    }
}

void TimeLineWidget::handleToolButtonNext_clicked() {
    int currentValue = ui->TimeSlider->value();
    if (currentValue < ui->TimeSlider->maximum()) {
        ui->TimeSlider->setValue(currentValue + ui->TimeSlider->singleStep());
    }
}

// =========== Изменение значения слайдера (перетаскивание) ===========
void TimeLineWidget::handleTimeSliderChangeValue(int value) {
    // TODO реализовать покадровое переключение
}

// =========== Изменение диапазона слайдера ===========
void TimeLineWidget::handleSnapshotsListChangeSize(int size) {
    if (size < 0) {
        ui->TimeSlider->setEnabled(false);
        ui->TimeSlider->setRange(0, 0);
        ui->TimeSlider->setValue(0);
        ui->label_Snapshot->setStyleSheet("QLabel {color: red;}");
        qCWarning(LogUI) << "unexpected size for slider: " << size;
        return;
    }

    ui->TimeSlider->setEnabled(true);
    ui->TimeSlider->setRange(1, size);
    ui->TimeSlider->setSingleStep(1);
    ui->TimeSlider->setPageStep(qMax(1, size / 10));
    ui->TimeSlider->setValue(1);
    setTextTimeLabelTextInternal();
}

// =========== Внутренние методы ===========
void TimeLineWidget::setTextTimeLabelTextInternal() {
    // ЗАЩИТА ОТ ВЫЛЕТОВ (Обязательно!)
    if (!m_app || !m_app->dataController())
        return;

    int    value     = ui->TimeSlider->value();
    double timestamp = m_app->dataController()->getSnapshotTimeByIndex(value - 1);

    QString snapshotString = QString("Снимок: %1/%2 (T=%3 миллионов лет)")
                                 .arg(value)
                                 .arg(ui->TimeSlider->maximum())
                                 .arg(timestamp);
    ui->label_Snapshot->setText(snapshotString);
}

TimeLineWidget::~TimeLineWidget() {
    delete ui;
}

} // namespace QSpace::UI