#include "PropertyInspector.h"
#include "ColorMapComboBox.h"
#include "ColorMapEditorDialog.h"
#include "Common/Structures/RenderStructures.h"
#include "Core/AppCore/AppCore.h"
#include "Core/AppCore/DataController.h"
#include "Core/AppCore/ProjectController.h"
#include "Enums/RenderEnums.h"
#include "Structures/CoreStructures.h"
#include "Visualize/ColorMapManager/ColorMapManager.h"
#include "ui_PropertyInspector.h"
#include <QSignalBlocker>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qicon.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qoverload.h>
#include <qslider.h>
#include <qspinbox.h>
#include <quuid.h>
#include <qvariant.h>

namespace QSpace::UI {
PropertyInspector::PropertyInspector(Core::AppCore* app, QWidget* parent)
    : QWidget(parent), ui(new Ui::PropertyInspector), m_app(app) {
    ui->setupUi(this);

    // Выключаем панель, пока ничего не выбрано
    this->setEnabled(false);
    // отключает установку диапазона (изначально он определяется автоматически)
    ui->spin_maxValue->setEnabled(false);
    ui->spin_minValue->setEnabled(false);
    setupUiLogic();
}

PropertyInspector::~PropertyInspector() {
    delete ui;
}

void PropertyInspector::setupUiLogic() {
    // 1. Заполняем палитры
    ui->combo_colormap->clear();
    ui->combo_colormap->setIconSize(QSize(50, 18));
    auto& manager = Visualize::ColorMapManager::instance();
    for (auto colorMap : manager.getAllMaps()) {
        QIcon icon = manager.createColorMapIcon(colorMap);
        ui->combo_colormap->addItem(icon,          // палитра
                                    colorMap.name, // название
                                    colorMap.id    // Прячем Enum в UserData
        );
    }
    ui->combo_RenderMode->clear();
    for (auto mode : QSpace::Visualize::getAllRenderModes()) {
        ui->combo_RenderMode->addItem(QSpace::Visualize::rendermodeToString(mode), QVariant::fromValue(mode));
    }
    ui->combo_interpolationRange->clear();
    for (auto type : QSpace::Visualize::getAllScalarBarRangeInterpolationTypes()) {
        ui->combo_interpolationRange->addItem(
            QSpace::Visualize::scalarBarRangeInterpolationTypeToString(type),
            QVariant::fromValue(type));
    }
    ui->combo_functionSplat->clear();
    for (auto type : QSpace::Visualize::getAllShaderTypes()) {
        ui->combo_functionSplat->addItem(QSpace::Visualize::shaderTypeToString(type),
                                         QVariant::fromValue(type));
    }

    ui->combo_functionOpacity->clear();
    for (auto type : QSpace::Visualize::getAllInterpolationOpacityFunctions()) {
        ui->combo_functionOpacity->addItem(QSpace::Visualize::interpolationOpacityFunctionToString(type),
                                           QVariant::fromValue(type));
    }
    //  2. Коннектим сигналы UI к слотамв
    connect(ui->combo_colormap,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertyInspector::handlePaletteChange);
    connect(ui->combo_colormap,
            &ColorMapComboBox::newButtonClicked,
            this,
            &PropertyInspector::handleCreateCustomPalete);
    connect(ui->combo_colormap,
            &ColorMapComboBox::editButtonClicked,
            this,
            &PropertyInspector::handleEditPalete);

    // Пример для слайдера прозрачности (если он есть в ui)
    connect(ui->doubleSpinBox_Opacity,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::handleOpacityChange);

    //(SpinBoxes)
    connect(ui->spin_maxValue,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::handleRangeMaxValueChange);
    connect(ui->spin_minValue,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::handleRangeMinValueChange);

    connect(ui->doubleSpinBox_ParticleSize,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::handleParticleSizeChange);
    connect(ui->combo_current_column,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertyInspector::handleCurrentVisualizeColumnChange);
    connect(ui->combo_RenderMode,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertyInspector::handleRenderModeChange);
    connect(ui->checkBox_useLogScale,
            &QCheckBox::toggled,
            this,
            &PropertyInspector::handleUseLogscaleToggled);
    connect(ui->checkBox_useEmissive, &QCheckBox::toggled, this, &PropertyInspector::handleUseEmisiveToggled);
    connect(ui->checkBox_isVisibleScalarBar,
            &QCheckBox::toggled,
            this,
            &PropertyInspector::handleShowScalarBarToggled);
    connect(ui->checkBox_isAutomaticRange,
            &QCheckBox::toggled,
            this,
            &PropertyInspector::handleAutoRangeToggled);
    connect(&Visualize::ColorMapManager::instance(),
            &Visualize::ColorMapManager::paleteAdded,
            this,
            &PropertyInspector::handleColorMapAdded);

    connect(ui->checkBox_hideOutOfRange,
            &QCheckBox::toggled,
            this,
            &PropertyInspector::handleHideOutOfRangeToggled);
    connect(ui->doubleSpinBox_colorCorrection,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::handleColorCorrectionChange);
    connect(ui->doubleSpinBox_gauianSharpnes,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::habdleGauianSharpnesChange);
    connect(ui->doubleSpinBox_sigmoidGammaOpacity,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::handleSigmoidGammaOpacityChange);
    connect(ui->doubleSpinBox_sigmoidGammaColor,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::handleSigmoidGammaColorChange);
    connect(ui->doubleSpinBox__sigmoidShiftOpacity,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::handleSigmoidShiftOpacityChange);
    connect(ui->doubleSpinBox__sigmoidShiftColor,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::handleSigmoidShiftColorChange);
    connect(ui->combo_functionSplat,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertyInspector::handleFunctionSplatChange);
    connect(ui->combo_interpolationRange,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertyInspector::handleInterpolationRangeChange);
    connect(ui->combo_functionOpacity,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertyInspector::handleFunctionOpacityChange);
    connect(ui->doubleSpinBox__alpha,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &PropertyInspector::handleAlphaChange);
}
void PropertyInspector::handleFunctionOpacityChange(int index) {
    if (m_currentNodeId.isNull())
        return;
    auto type = ui->combo_functionOpacity->itemData(index).value<Visualize::InterpolationOpacityFunction>();
    if (type == Visualize::InterpolationOpacityFunction::Sigmoid) {
        ui->doubleSpinBox_sigmoidGammaOpacity->setEnabled(true);
        ui->doubleSpinBox__sigmoidShiftOpacity->setEnabled(true);
    } else {
        ui->doubleSpinBox_sigmoidGammaOpacity->setEnabled(false);
        ui->doubleSpinBox__sigmoidShiftOpacity->setEnabled(false);
    }
    m_app->dataController()->updateNodeSettings(m_currentNodeId, [type](Core::VisualSettings& s) {
        s.interpolationOpacityFunction = Visualize::interpolationOpacityFunctionToString(type);
    });
}

void PropertyInspector::handleColorMapAdded(const Visualize::ColorMap& map) {
    auto& manager = Visualize::ColorMapManager::instance();
    QIcon icon    = manager.createColorMapIcon(map);
    ui->combo_colormap->removeItem(
        ui->combo_colormap->findData(map.id)); // Удаляем старую запись, если она есть
    ui->combo_colormap->addItem(icon, map.name, map.id);
    // 6. Обновляем комбобокс без срабатывания сигналов
    QSignalBlocker blocker(ui->combo_colormap);
    // Выбираем свежедобавленную палитру
    ui->combo_colormap->setCurrentIndex(ui->combo_colormap->count() - 1);
    // 7. Обновляем ядро и рендер
    auto colorMapId = map.id;
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [colorMapId](QSpace::Core::VisualSettings& s) { s.colorMapId = colorMapId; });
}
void PropertyInspector::handleCreateCustomPalete() {
    if (m_currentNodeId.isNull())
        return;

    // 1. Берем текущую палитру как базовую (чтобы было что редактировать)
    auto paleteId   = m_app->dataController()->getNodePaletteId(m_currentNodeId);
    auto baseMapOpt = QSpace::Visualize::ColorMapManager::instance().getMap(paleteId);
    if (!baseMapOpt.has_value()) {
        QMessageBox::warning(this, "Ошибка", "Текущая палитра не найдена. Невозможно создать на ее основе.");
        return;
    }
    // Если палитра не найдена, берем первую дефолтную
    Visualize::ColorMap baseMap = baseMapOpt.has_value()
                                      ? baseMapOpt.value()
                                      : Visualize::ColorMapManager::instance().getAllMaps().first();

    // 2. Создаем диалог, передавая базовую палитру
    ColorMapEditorDialog dialog(baseMap, this);
    connect(&dialog,
            &ColorMapEditorDialog::savePaletteRequested,
            m_app->projectController(),
            &QSpace::Core::Controllers::ProjectController::savePalette);
    connect(&dialog,
            &ColorMapEditorDialog::loadPaletteRequested,
            m_app->projectController(),
            &QSpace::Core::Controllers::ProjectController::loadPalette);
    connect(m_app->projectController(),
            &QSpace::Core::Controllers::ProjectController::paletteLoaded,
            &dialog,
            &ColorMapEditorDialog::onPaletteLoaded);
    // 3. Открываем окно и ждем. Выполнение кода здесь останавливается,
    // пока пользователь не нажмет Ok или Cancel
    if (dialog.exec() == QDialog::Accepted) {
        // 4. Забираем готовую палитру из окна
        Visualize::ColorMap customMap = dialog.getEditedMap();

        // 5. Сохраняем в менеджер
        auto& manager = QSpace::Visualize::ColorMapManager::instance();
        manager.AddCustomMap(customMap);
    }
}
void PropertyInspector::handleEditPalete() {
    if (m_currentNodeId.isNull())
        return;

    auto nodeId     = m_app->dataController()->getNodePaletteId(m_currentNodeId);
    auto baseMapOpt = QSpace::Visualize::ColorMapManager::instance().getMap(nodeId);

    if (!baseMapOpt.has_value()) {
        QMessageBox::warning(this, "Ошибка", "Текущая палитра не найдена. Невозможно отредактировать.");
        return;
    }
    if (baseMapOpt.value().isPreset) {
        QMessageBox::warning(this,
                             "Ошибка",
                             "Невозможно редактировать пресет. Создайте копию палитры и редактируйте ее.");
        return;
    }
    Visualize::ColorMap baseMap = baseMapOpt.value();

    ColorMapEditorDialog dialog(baseMap, this);
    connect(&dialog,
            &ColorMapEditorDialog::savePaletteRequested,
            m_app->projectController(),
            &QSpace::Core::Controllers::ProjectController::savePalette);
    connect(&dialog,
            &ColorMapEditorDialog::loadPaletteRequested,
            m_app->projectController(),
            &QSpace::Core::Controllers::ProjectController::loadPalette);
    connect(m_app->projectController(),
            &QSpace::Core::Controllers::ProjectController::paletteLoaded,
            &dialog,
            &ColorMapEditorDialog::onPaletteLoaded);
    if (dialog.exec() == QDialog::Accepted) {
        Visualize::ColorMap editedMap = dialog.getEditedMap();
        // Если палитра была загружена из файла, сохраняем ее ID и имя
        editedMap.id       = baseMap.id;
        editedMap.name     = baseMap.name;
        editedMap.isPreset = false;

        auto& manager = QSpace::Visualize::ColorMapManager::instance();
        manager.AddCustomMap(editedMap);
    }
}
void PropertyInspector::handleShowScalarBarToggled(bool show) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(m_currentNodeId,
                                                [show](Core::VisualSettings& s) { s.showScalarBar = show; });
}

void PropertyInspector::handleAutoRangeToggled(bool use) {
    if (m_currentNodeId.isNull())
        return;
    if (use) {
        ui->spin_maxValue->setEnabled(false);
        ui->spin_minValue->setEnabled(false);
    } else {
        ui->spin_maxValue->setEnabled(true);
        ui->spin_minValue->setEnabled(true);
    }
    m_app->dataController()->updateNodeSettings(m_currentNodeId,
                                                [use](Core::VisualSettings& s) { s.autoRange = use; });
}

void PropertyInspector::handleCurrentVisualizeColumnChange(int index) {
    if (m_currentNodeId.isNull())
        return;

    QString fieldName = ui->combo_current_column->itemText(index);
    m_app->dataController()->updateNodeSettings(m_currentNodeId, [fieldName](Core::VisualSettings& s) {
        s.colorByField = fieldName;
    });
    updateWidgets();
}
void PropertyInspector::handleParticleSizeChange(double val) {
    if (m_currentNodeId.isNull())
        return;
    double size = val;
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
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

void PropertyInspector::updateWidgets() {
    auto node = m_app->dataController()->getNodeById(m_currentNodeId);
    if (!node)
        return;

    // Блокируем сигналы, чтобы установка значений в UI не вызвала обратную команду в Core
    QSignalBlocker blocker(this);
    QSignalBlocker comboBlocker(ui->combo_colormap);
    QSignalBlocker comboBlocker1(ui->combo_current_column);
    QSignalBlocker comboBlocker2(ui->combo_RenderMode);
    QSignalBlocker spinBlocker(ui->doubleSpinBox_Opacity);
    QSignalBlocker spinBlocker1(ui->doubleSpinBox_ParticleSize);
    QSignalBlocker checkBlocker(ui->checkBox_isAutomaticRange);
    QSignalBlocker spinBlocker2(ui->spin_maxValue);
    QSignalBlocker spinBlocker3(ui->spin_minValue);
    QSignalBlocker spinBlocker4(ui->checkBox_useLogScale);
    QSignalBlocker spinBlocker5(ui->checkBox_useEmissive);
    QSignalBlocker spinBlocker6(ui->checkBox_isVisibleScalarBar);
    QSignalBlocker spinBlocker7(ui->checkBox_hideOutOfRange);
    QSignalBlocker spinBlocker8(ui->doubleSpinBox_colorCorrection);
    QSignalBlocker spinBlocker9(ui->doubleSpinBox_gauianSharpnes);
    QSignalBlocker spinBlocker10(ui->doubleSpinBox_sigmoidGammaOpacity);
    QSignalBlocker spinBlocker11(ui->doubleSpinBox_sigmoidGammaColor);
    QSignalBlocker spinBlocker12(ui->doubleSpinBox__sigmoidShiftOpacity);
    QSignalBlocker spinBlocker13(ui->doubleSpinBox__sigmoidShiftColor);
    QSignalBlocker spinBlocker14(ui->combo_functionSplat);
    QSignalBlocker spinBlocker15(ui->combo_interpolationRange);
    QSignalBlocker spinBlocker16(ui->combo_functionOpacity);
    QSignalBlocker spinBlocker17(ui->doubleSpinBox__alpha);

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
    index = ui->combo_current_column->findText(node->masterSettings->colorByField);
    if (index != -1) {
        ui->combo_current_column->setCurrentIndex(index);
    }
    // Устанавливаем текущую палитру в комбобоксе
    index = ui->combo_colormap->findData(QVariant::fromValue(node->masterSettings->colorMapId));
    if (index != -1) {
        ui->combo_colormap->setCurrentIndex(index);
    }
    index = ui->combo_RenderMode->findData(QVariant::fromValue(node->masterSettings->mode));
    if (index != -1) {
        ui->combo_RenderMode->setCurrentIndex(index);
    }
    // Здесь же обновляем другие виджеты (чекбоксы, слайдеры...)
    ui->doubleSpinBox_Opacity->setValue(node->masterSettings->opacity);
    ui->doubleSpinBox_ParticleSize->setValue(node->masterSettings->PointSize);
    ui->checkBox_isAutomaticRange->setCheckState(node->masterSettings->autoRange ? Qt::CheckState::Checked
                                                                                 : Qt::CheckState::Unchecked);
    if (node->masterSettings->autoRange) {
        ui->spin_maxValue->setEnabled(false);
        ui->spin_minValue->setEnabled(false);
    } else {
        ui->spin_maxValue->setEnabled(true);
        ui->spin_minValue->setEnabled(true);
    }
    ui->spin_maxValue->setMaximum(node->masterSettings->baseRangeMax);
    ui->spin_maxValue->setMinimum(node->masterSettings->baseRangeMin);
    ui->spin_minValue->setMaximum(node->masterSettings->baseRangeMax);
    ui->spin_minValue->setMinimum(node->masterSettings->baseRangeMin);

    if (node->masterSettings->autoRange) {
        ui->spin_maxValue->setValue(node->masterSettings->rangeMax);
        ui->spin_minValue->setValue(node->masterSettings->rangeMin);
    }
    ui->checkBox_useLogScale->setCheckState(node->masterSettings->useLogScale ? Qt::CheckState::Checked
                                                                              : Qt::CheckState::Unchecked);
    ui->checkBox_useEmissive->setCheckState(node->masterSettings->isEmmisive ? Qt::CheckState::Checked
                                                                             : Qt::CheckState::Unchecked);
    ui->checkBox_isVisibleScalarBar->setCheckState(
        node->masterSettings->showScalarBar ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->checkBox_hideOutOfRange->setCheckState(
        node->masterSettings->hideOutOfRange ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    ui->doubleSpinBox_colorCorrection->setValue(node->masterSettings->exposureClamp);
    ui->doubleSpinBox_gauianSharpnes->setValue(node->masterSettings->gaussianSharpness);
    ui->doubleSpinBox_sigmoidGammaOpacity->setValue(node->masterSettings->sigmoidGammaOpacity);
    ui->doubleSpinBox__sigmoidShiftOpacity->setValue(node->masterSettings->sigmoidShiftOpacity);
    ui->doubleSpinBox_sigmoidGammaColor->setValue(node->masterSettings->sigmoidGammaColor);
    ui->doubleSpinBox__sigmoidShiftColor->setValue(node->masterSettings->sigmoidShiftColor);
    ui->doubleSpinBox__alpha->setValue(node->masterSettings->alpha);

    index = ui->combo_functionSplat->findData(QVariant::fromValue(node->masterSettings->ShaderType));
    if (index != -1)
        ui->combo_functionSplat->setCurrentIndex(index);

    index = ui->combo_interpolationRange->findData(
        QVariant::fromValue(node->masterSettings->interpolationRangeType));
    if (index != -1)
        ui->combo_interpolationRange->setCurrentIndex(index);

    auto shader = ui->combo_functionSplat->currentData().value<Visualize::ShaderType>();
    if (shader == Visualize::ShaderType::Default) {
        ui->doubleSpinBox_colorCorrection->setEnabled(false);
        ui->doubleSpinBox_gauianSharpnes->setEnabled(false);
    } else {
        ui->doubleSpinBox_colorCorrection->setEnabled(true);
        ui->doubleSpinBox_gauianSharpnes->setEnabled(true);
    }
    auto type = ui->combo_functionOpacity->currentData().value<Visualize::InterpolationOpacityFunction>();
    if (type == Visualize::InterpolationOpacityFunction::Sigmoid) {
        ui->doubleSpinBox_sigmoidGammaOpacity->setEnabled(true);
        ui->doubleSpinBox__sigmoidShiftOpacity->setEnabled(true);
    } else {
        ui->doubleSpinBox_sigmoidGammaOpacity->setEnabled(false);
        ui->doubleSpinBox__sigmoidShiftOpacity->setEnabled(false);
    }
    auto mode =
        ui->combo_interpolationRange->currentData().value<QSpace::Visualize::ScalarBarRangeInterpolation>();
    if (mode == Visualize::ScalarBarRangeInterpolation::Sigmoid) {
        ui->doubleSpinBox_sigmoidGammaColor->setEnabled(true);
        ui->doubleSpinBox__sigmoidShiftColor->setEnabled(true);
    } else {
        ui->doubleSpinBox_sigmoidGammaColor->setEnabled(false);
        ui->doubleSpinBox__sigmoidShiftColor->setEnabled(false);
    }
    if (mode == Visualize::ScalarBarRangeInterpolation::Asinh) {
        ui->doubleSpinBox__alpha->setEnabled(true);
    } else {
        ui->doubleSpinBox__alpha->setEnabled(false);
    }
}
void PropertyInspector::handleHideOutOfRangeToggled(bool checked) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [checked](QSpace::Core::VisualSettings& settings) { settings.hideOutOfRange = checked; });
}
void PropertyInspector::handleColorCorrectionChange(double val) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [val](QSpace::Core::VisualSettings& settings) { settings.exposureClamp = val; });
}
void PropertyInspector::habdleGauianSharpnesChange(double val) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [val](QSpace::Core::VisualSettings& settings) { settings.gaussianSharpness = val; });
}
void PropertyInspector::handleSigmoidGammaOpacityChange(double val) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [val](QSpace::Core::VisualSettings& settings) { settings.sigmoidGammaOpacity = val; });
}
void PropertyInspector::handleSigmoidShiftOpacityChange(double val) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [val](QSpace::Core::VisualSettings& settings) { settings.sigmoidShiftOpacity = val; });
}
void PropertyInspector::handleSigmoidGammaColorChange(double val) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [val](QSpace::Core::VisualSettings& settings) { settings.sigmoidGammaColor = val; });
}
void PropertyInspector::handleSigmoidShiftColorChange(double val) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [val](QSpace::Core::VisualSettings& settings) { settings.sigmoidShiftColor = val; });
}
void PropertyInspector::handleAlphaChange(double val) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [val](QSpace::Core::VisualSettings& settings) { settings.alpha = val; });
}
void PropertyInspector::handleFunctionSplatChange(int index) {
    if (m_currentNodeId.isNull())
        return;
    auto type = ui->combo_functionSplat->itemData(index).value<QSpace::Visualize::ShaderType>();
    if (type == Visualize::ShaderType::Default) {
        ui->doubleSpinBox_gauianSharpnes->setEnabled(false);
        ui->doubleSpinBox_colorCorrection->setEnabled(false);
    } else {
        ui->doubleSpinBox_gauianSharpnes->setEnabled(true);
        ui->doubleSpinBox_colorCorrection->setEnabled(true);
    }
    m_app->dataController()->updateNodeSettings(m_currentNodeId,
                                                [type](QSpace::Core::VisualSettings& settings) {
                                                    settings.ShaderType = Visualize::shaderTypeToString(type);
                                                });
}
void PropertyInspector::handleInterpolationRangeChange(int index) {
    if (m_currentNodeId.isNull())
        return;
    auto mode =
        ui->combo_interpolationRange->itemData(index).value<QSpace::Visualize::ScalarBarRangeInterpolation>();
    if (mode == Visualize::ScalarBarRangeInterpolation::Sigmoid) {
        ui->doubleSpinBox_sigmoidGammaColor->setEnabled(true);
        ui->doubleSpinBox__sigmoidShiftColor->setEnabled(true);
    } else {
        ui->doubleSpinBox_sigmoidGammaColor->setEnabled(false);
        ui->doubleSpinBox__sigmoidShiftColor->setEnabled(false);
    }
    if (mode == Visualize::ScalarBarRangeInterpolation::Asinh) {
        ui->doubleSpinBox__alpha->setEnabled(true);
    } else {
        ui->doubleSpinBox__alpha->setEnabled(false);
    }
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [mode](QSpace::Core::VisualSettings& settings) {
            settings.interpolationRangeType = Visualize::scalarBarRangeInterpolationTypeToString(mode);
        });
}
void PropertyInspector::handleRangeMaxValueChange(double maxVal) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [maxVal](QSpace::Core::VisualSettings& settings) { settings.rangeMax = maxVal; });
}
void PropertyInspector::handleRangeMinValueChange(double minVal) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [minVal](QSpace::Core::VisualSettings& settings) { settings.rangeMin = minVal; });
}

void PropertyInspector::handlePaletteChange(int index) {
    if (m_currentNodeId.isNull())
        return;

    // Достаем тип палитры из выбранного пункта
    auto colorMapId = ui->combo_colormap->itemData(index).value<QUuid>();

    // Обновляем Core (PipelineManager поймает это изменение сам)
    m_app->dataController()->updateNodeSettings(
        m_currentNodeId,
        [colorMapId](QSpace::Core::VisualSettings& s) { s.colorMapId = colorMapId; });
}
void PropertyInspector::handleUseLogscaleToggled(bool use) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(m_currentNodeId, [use](QSpace::Core::VisualSettings& s) {
        s.useLogScale = use;
    });
    auto   node = m_app->dataController()->getNodeById(m_currentNodeId);
    double minV = node->masterSettings->baseRangeMin;
    double maxV = node->masterSettings->baseRangeMax;
    ui->spin_maxValue->setMaximum(maxV);
    ui->spin_minValue->setMaximum(maxV);
    ui->spin_maxValue->setMinimum(minV);
    ui->spin_minValue->setMinimum(minV);
    ui->spin_maxValue->setValue(maxV);
    ui->spin_minValue->setValue(minV);
}
void PropertyInspector::handleUseEmisiveToggled(bool use) {
    if (m_currentNodeId.isNull())
        return;
    m_app->dataController()->updateNodeSettings(m_currentNodeId, [use](QSpace::Core::VisualSettings& s) {
        s.isEmmisive = use;
    });
}

// изменяет тип отображения всего их 3: Points, GausianSplat,Volume -- в разработке
void PropertyInspector::handleRenderModeChange(int index) {
    if (m_currentNodeId.isNull())
        return;
    auto mode = ui->combo_RenderMode->itemData(index).value<QSpace::Visualize::RenderMode>();
    m_app->dataController()->updateNodeSettings(m_currentNodeId,
                                                [mode](QSpace::Core::VisualSettings& s) { s.mode = mode; });
}
void PropertyInspector::handleOpacityChange(double val) {
    if (m_currentNodeId.isNull())
        return;

    double opacity = val;
    m_app->dataController()->updateNodeSettings(m_currentNodeId, [opacity](QSpace::Core::VisualSettings& s) {
        s.opacity = opacity;
    });
}
} // namespace QSpace::UI