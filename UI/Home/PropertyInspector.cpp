#include "PropertyInspector.h"
#include "Common/Structures/RenderStructures.h" // Твой класс с палитрами
#include "Structures/CoreStructures.h"
#include "ui_PropertyInspector.h" // Генерируется из твоего нового .ui
#include <QSignalBlocker>
#include <qcombobox.h>
#include <qobject.h>
#include <qoverload.h>
#include <qslider.h>

namespace QSpace::UI {
PropertyInspector::PropertyInspector(Core::AppCore* app, QWidget* parent)
    : QWidget(parent), ui(new Ui::PropertyInspector), m_app(app) {
    ui->setupUi(this);

    // Выключаем панель, пока ничего не выбрано
    this->setEnabled(false);

    setupUiLogic();
}

PropertyInspector::~PropertyInspector() {
    delete ui;
}

void PropertyInspector::setupUiLogic() {
    // 1. Заполняем палитры
    ui->combo_colormap->clear();
    for (auto type : QSpace::Visualize::ColorMapRegistry::getAllTypes()) {
        ui->combo_colormap->addItem(QSpace::Visualize::ColorMapRegistry::toString(type),
                                    QVariant::fromValue(type) // Прячем Enum в UserData
        );
    }

    // 2. Коннектим сигналы UI к слотам
    connect(ui->combo_colormap,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertyInspector::onPaletteChanged);

    // Пример для слайдера прозрачности (если он есть в ui)
    connect(ui->slider_opacity, &QSlider::valueChanged, this, &PropertyInspector::onOpacityChanged);

    // Alpha / Beta (SpinBoxes)
    connect(ui->spin_alpha,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::onTransformChanged);
    connect(ui->spin_beta,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::onTransformChanged);
    connect(ui->slider_size, &QSlider::valueChanged, this, &PropertyInspector::onParticleSizeChanged);
    connect(ui->combo_current_column,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertyInspector::onCurrentVisColumnChanged);
}
void PropertyInspector::onCurrentVisColumnChanged(int index) {
    if (m_currentNodeId.isNull())
        return;

    QString fieldName = ui->combo_current_column->itemText(index);
    m_app->updateNodeSettings(m_currentNodeId, [fieldName](Core::VisualSettings& s) { s.colorByField = fieldName; });
}
void PropertyInspector::onParticleSizeChanged(int value) {
    if (m_currentNodeId.isNull())
        return;
    double size = value * 0.0005;
    m_app->updateNodeSettings(m_currentNodeId,
                              [size](QSpace::Core::VisualSettings& settings) { settings.PointSize = size; });
}
void PropertyInspector::setCurrentNode(const QUuid& id) {
    m_currentNodeId = id;

    if (id.isNull()) {
        this->setEnabled(false);
        return;
    }

    this->setEnabled(true);
    updateWidgets();
}
void PropertyInspector::onTransformChanged(double val) {
    // Общий слот для спинбоксов, если нужно различать отправителя:
    auto senderBox = qobject_cast<QDoubleSpinBox*>(sender());
    if (!senderBox || m_currentNodeId.isNull())
        return;

    m_app->updateNodeSettings(m_currentNodeId, [this, senderBox](Core::VisualSettings& s) {
        if (senderBox == ui->spin_alpha)
            s.alpha = senderBox->value();
        if (senderBox == ui->spin_beta)
            s.beta = senderBox->value();
    });
}
void PropertyInspector::updateWidgets() {
    auto node = m_app->getObjectRegistry()->getNode(m_currentNodeId);
    if (!node)
        return;

    // Блокируем сигналы, чтобы установка значений в UI не вызвала обратную команду в Core
    QSignalBlocker blocker(this);
    QSignalBlocker comboBlocker(ui->combo_colormap);
    QSignalBlocker comboBlocker1(ui->combo_current_column);
    QSignalBlocker sliderBlocker(ui->slider_opacity);

    ui->combo_current_column->clear();
    auto pointData = node->data->GetPointData();
    int  numArrays = pointData->GetNumberOfArrays();
    for (int i = 0; i < numArrays; ++i) {
        vtkDataArray* array = pointData->GetArray(i);
        if (array) {
            QString arrayName = array->GetName();
            if (!arrayName.isEmpty()) {
                ui->combo_current_column->addItem(arrayName);
            }
        }
    }
    int index;
    // Устанавливаем текущее выбранное поле из настроек ноды
    index = ui->combo_current_column->findText(node->settings.colorByField);
    if (index != -1) {
        ui->combo_current_column->setCurrentIndex(index);
    }
    // Устанавливаем текущую палитру в комбобоксе
    index = ui->combo_colormap->findData(QVariant::fromValue(node->settings.colorMap));
    if (index != -1) {
        ui->combo_colormap->setCurrentIndex(index);
    }

    // Здесь же обновляем другие виджеты (чекбоксы, слайдеры...)
    ui->slider_opacity->setValue(100 * (1.0 - node->settings.opacity));
    ui->slider_size->setValue(node->settings.PointSize * 2000);
}

void PropertyInspector::onPaletteChanged(int index) {
    if (m_currentNodeId.isNull())
        return;

    // Достаем тип палитры из выбранного пункта
    auto type = ui->combo_colormap->itemData(index).value<QSpace::Visualize::ColorMapType>();

    // Обновляем Core (PipelineManager поймает это изменение сам)
    m_app->updateNodeSettings(m_currentNodeId, [type](QSpace::Core::VisualSettings& s) { s.colorMap = type; });
}

void PropertyInspector::onOpacityChanged(int value) {
    if (m_currentNodeId.isNull())
        return;

    double opacity = 1.0 - value / 100.0;
    m_app->updateNodeSettings(m_currentNodeId, [opacity](QSpace::Core::VisualSettings& s) { s.opacity = opacity; });
}
} // namespace QSpace::UI