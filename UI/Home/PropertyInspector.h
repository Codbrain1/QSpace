#pragma once
#include "ColorMapComboBox.h"
#include "Core/AppCore/AppCore.h"
#include <QMainWindow>
#include <QObject>
#include <QPushButton>
#include <QVTKOpenGLNativeWidget.h>
#include <QWidget>
#include <qcontainerfwd.h>
#include <qdockwidget.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qmainwindow.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qtmetamacros.h>
#include <vtkDataSetAttributes.h>
#include <vtkType.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class PropertyInspector;
}
QT_END_NAMESPACE

namespace QSpace::UI {
class PropertyInspector : public QWidget {
    Q_OBJECT
  public:
    explicit PropertyInspector(QSpace::Core::AppCore* app, QWidget* parent = nullptr);
    ~PropertyInspector();
    void setCurrentNode(const QUuid& id);
  private slots:
    // Слот для комбобокса палитр
    void handlePaletteChange(int index);
    void handleCurrentVisualizeColumnChange(int index);
    void handleRenderModeChange(int index);

    // Слот для прозрачности (например, если есть слайдер)
    void handleOpacityChange(double val);
    void handleParticleSizeChange(double val);
    void handleUseLogscaleToggled(bool use);
    void handleUseEmisiveToggled(bool use);
    void handleRangeMaxValueChange(double maxVal);
    void handleRangeMinValueChange(double minVal);
    void handleAutoRangeToggled(bool use);
    void handleShowScalarBarToggled(bool show);
    void handleCreateCustomPalete();
    void handleEditPalete();
    void handleColorMapAdded(const Visualize::ColorMap& map);
    void handleHideOutOfRangeToggled(bool use);
    void handleColorCorrectionChange(double val);
    void habdleGauianSharpnesChange(double val);
    void handleSigmoidGammaOpacityChange(double val);
    void handleSigmoidGammaColorChange(double val);
    void handleSigmoidShiftOpacityChange(double val);
    void handleSigmoidShiftColorChange(double val);
    void handleFunctionSplatChange(int index);
    void handleInterpolationRangeChange(int index);
    void handleFunctionOpacityChange(int index);
    void handleAlphaChange(double val);

  private:
    Ui::PropertyInspector* ui;
    QUuid                  m_currentNodeId;
    QSpace::Core::AppCore* m_app;
    void                   setupUiLogic();  // Начальное заполнение комбобоксов
    void                   updateWidgets(); // Синхронизация UI с данными ноды
};
} // namespace QSpace::UI