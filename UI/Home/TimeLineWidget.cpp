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
            &TimeLineWidget::handleSnapshotsSizeChange);

    m_updateTimer.setSingleShot(true);
    m_updateTimer.setInterval(50); // 50мс дает более плавный отклик UI

    // 1. Обработка изменения значения (перетаскивание или программный ввод)
    connect(ui->TimeSlider, &QSlider::valueChanged, this, &TimeLineWidget::handleTimeSlider_valueChanged);

    // 2. НОВОЕ: Обработка отпускания ползунка (финальная тяжелая загрузка)
    connect(ui->TimeSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_app || !m_app->dataController())
            return;

        m_updateTimer.stop(); // Глушим таймер превью
        int currentValue = ui->TimeSlider->value();

        // Принудительно вызываем честную загрузку
        m_app->dataController()->handleTimeSliderValueChanged(currentValue, false);
        m_pendingSliderValue = -1;
    });

    // 3. Работа таймера превью
    connect(&m_updateTimer, &QTimer::timeout, this, [this]() {
        if (m_pendingSliderValue != -1 && m_app && m_app->dataController()) {
            // Поскольку таймер работает только при зажатом ползунке, всегда передаем true
            m_app->dataController()->handleTimeSliderValueChanged(m_pendingSliderValue, true);
        }
    });

    connect(ui->toolButtonPrev, &QToolButton::clicked, this, &TimeLineWidget::handleToolButtonPrev_clicked);
    connect(ui->toolButtonNext, &QToolButton::clicked, this, &TimeLineWidget::handleToolButtonNext_clicked);
}

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

void TimeLineWidget::handleSnapshotsSizeChange(int size) {
    if (size <= 0) {
        ui->TimeSlider->setEnabled(false);
        ui->TimeSlider->setRange(0, 0);
        ui->TimeSlider->setValue(0);
        qCWarning(LogUI) << "unexpected size for slider: " << size;
        return;
    }

    ui->TimeSlider->setEnabled(true);
    ui->TimeSlider->setRange(0, size - 1);
    ui->TimeSlider->setSingleStep(1);
    ui->TimeSlider->setPageStep(qMax(1, size / 10));
    ui->TimeSlider->setValue(0);
    setTimeLabelTextInternal();
}

void TimeLineWidget::handleTimeSlider_valueChanged(int value) {
    // ЗАЩИТА ОТ ВЫЛЕТОВ (Обязательно!)
    if (!m_app || !m_app->dataController())
        return;

    setTimeLabelTextInternal();
    m_pendingSliderValue = value;

    if (ui->TimeSlider->isSliderDown()) {
        // Пользователь тянет ползунок. Запускаем/перезапускаем таймер для превью.
        if (!m_updateTimer.isActive()) {
            m_updateTimer.start();
        }
    } else {
        // Значение изменилось НЕ от перетаскивания (клик по треку, стрелки клавиатуры, кнопки Prev/Next)
        m_updateTimer.stop();
        m_app->dataController()->handleTimeSliderValueChanged(value, false);
        m_pendingSliderValue = -1;
    }
}

void TimeLineWidget::setTimeLabelTextInternal() {
    // ЗАЩИТА ОТ ВЫЛЕТОВ (Обязательно!)
    if (!m_app || !m_app->dataController())
        return;

    int    value     = ui->TimeSlider->value();
    double timestamp = m_app->dataController()->getSnapshotTimeByIndex(value);

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