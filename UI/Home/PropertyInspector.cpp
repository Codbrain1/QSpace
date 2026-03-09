#include "PropertyInspector.h"
#include "Common/Structures/RenderStructures.h" // Твой класс с палитрами
#include "Enums/RenderEnums.h"
#include "Structures/CoreStructures.h"
#include "ui_PropertyInspector.h" // Генерируется из твоего нового .ui
#include <QSignalBlocker>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qoverload.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qvariant.h>

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
    ui->combo_RenderMode->clear();
    for (auto mode : QSpace::Visualize::getAllRenderModes()) {
        ui->combo_RenderMode->addItem(QSpace::Visualize::rendermodeToString(mode), QVariant::fromValue(mode));
    }
    // 2. Коннектим сигналы UI к слотам
    connect(ui->combo_colormap,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertyInspector::onPaletteChanged);

    // Пример для слайдера прозрачности (если он есть в ui)
    connect(ui->doubleSpinBox_Opacity,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::onOpacityChanged);

    // Alpha / Beta (SpinBoxes)
    connect(ui->spin_alpha,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::onTransformChanged);
    connect(ui->spin_beta,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::onTransformChanged);

    connect(ui->spin_maxValue,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::onRangeMaxValueChanged);
    connect(ui->spin_minValue,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::onRangeMinValueChanged);

    connect(ui->doubleSpinBox_ParticleSize,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::onParticleSizeChanged);
    connect(ui->combo_current_column,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertyInspector::onCurrentVisColumnChanged);
    connect(ui->combo_RenderMode,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertyInspector::onRenderModeChanged);
    connect(ui->checkBox_useLogScale, &QCheckBox::toggled, this, &PropertyInspector::onUseLogscaleChanged);
    connect(ui->checkBox_useEmissive, &QCheckBox::toggled, this, &PropertyInspector::onUseEmisiveChanged);
    connect(ui->checkBox_isAutomaticRange, &QCheckBox::toggled, this, &PropertyInspector::onCheckBoxAutoRangeChanged);
}
void PropertyInspector::onCheckBoxAutoRangeChanged(bool checked) {
    if (m_currentNodeId.isNull())
        return;
    if (checked) {
        ui->spin_maxValue->setEnabled(false);
        ui->spin_minValue->setEnabled(false);
    } else {
        ui->spin_maxValue->setEnabled(true);
        ui->spin_minValue->setEnabled(true);
    }
    m_app->updateNodeSettings(m_currentNodeId, [checked](Core::VisualSettings& s) { s.autoRange = checked; });
}
void PropertyInspector::onCurrentVisColumnChanged(int index) {
    if (m_currentNodeId.isNull())
        return;

    QString fieldName = ui->combo_current_column->itemText(index);
    m_app->updateNodeSettings(m_currentNodeId, [fieldName](Core::VisualSettings& s) { s.colorByField = fieldName; });
}
void PropertyInspector::onParticleSizeChanged(double val) {
    if (m_currentNodeId.isNull())
        return;
    double size = val;
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
    QSignalBlocker comboBlocker2(ui->combo_RenderMode);
    QSignalBlocker spinBlocker(ui->doubleSpinBox_ParticleSize);
    QSignalBlocker spinBlocker1(ui->doubleSpinBox_Opacity);

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
    index = ui->combo_RenderMode->findData(QVariant::fromValue(node->settings.mode));
    if (index != -1) {
        ui->combo_RenderMode->setCurrentIndex(index);
    }
    // Здесь же обновляем другие виджеты (чекбоксы, слайдеры...)
    ui->doubleSpinBox_Opacity->setValue(node->settings.opacity);
    ui->doubleSpinBox_ParticleSize->setValue(node->settings.PointSize);
    ui->checkBox_isAutomaticRange->setCheckState(node->settings.autoRange ? Qt::CheckState::Checked
                                                                          : Qt::CheckState::Unchecked);
    if (node->settings.autoRange) {
        ui->spin_maxValue->setValue(node->settings.rangeMax);
        ui->spin_minValue->setValue(node->settings.rangeMin);
    }
    ui->checkBox_useLogScale->setCheckState(node->settings.useLogScale ? Qt::CheckState::Checked
                                                                       : Qt::CheckState::Unchecked);
    ui->checkBox_useEmissive->setCheckState(node->settings.isEmmisive ? Qt::CheckState::Checked
                                                                      : Qt::CheckState::Unchecked);
}
void PropertyInspector::onRangeMaxValueChanged(double val) {
    if (m_currentNodeId.isNull())
        return;
    m_app->updateNodeSettings(m_currentNodeId,
                              [val](QSpace::Core::VisualSettings& settings) { settings.rangeMax = val; });
}
void PropertyInspector::onRangeMinValueChanged(double val) {
    if (m_currentNodeId.isNull())
        return;
    m_app->updateNodeSettings(m_currentNodeId,
                              [val](QSpace::Core::VisualSettings& settings) { settings.rangeMin = val; });
}

void PropertyInspector::onPaletteChanged(int index) {
    if (m_currentNodeId.isNull())
        return;

    // Достаем тип палитры из выбранного пункта
    auto type = ui->combo_colormap->itemData(index).value<QSpace::Visualize::ColorMapType>();

    // Обновляем Core (PipelineManager поймает это изменение сам)
    m_app->updateNodeSettings(m_currentNodeId, [type](QSpace::Core::VisualSettings& s) { s.colorMap = type; });
}
void PropertyInspector::onUseLogscaleChanged(bool checked) {
    if (m_currentNodeId.isNull())
        return;
    m_app->updateNodeSettings(m_currentNodeId, [checked](QSpace::Core::VisualSettings& s) { s.useLogScale = checked; });
}
void PropertyInspector::onUseEmisiveChanged(bool checked) {
    if (m_currentNodeId.isNull())
        return;
    m_app->updateNodeSettings(m_currentNodeId, [checked](QSpace::Core::VisualSettings& s) { s.isEmmisive = checked; });
}

// изменяет тип отображения всего их 3: Points, GausianSplat,Volume -- в разработке
void PropertyInspector::onRenderModeChanged(int index) {
    if (m_currentNodeId.isNull())
        return;
    auto mode = ui->combo_RenderMode->itemData(index).value<QSpace::Visualize::RenderMode>();
    m_app->updateNodeSettings(m_currentNodeId, [mode](QSpace::Core::VisualSettings& s) { s.mode = mode; });
}
void PropertyInspector::onOpacityChanged(double val) {
    if (m_currentNodeId.isNull())
        return;

    double opacity = val;
    m_app->updateNodeSettings(m_currentNodeId, [opacity](QSpace::Core::VisualSettings& s) { s.opacity = opacity; });
}
} // namespace QSpace::UI