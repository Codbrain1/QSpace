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
    void onPaletteChanged(int index);
    void onCurrentVisColumnChanged(int index);
    void onRenderModeChanged(int index);

    // Слот для прозрачности (например, если есть слайдер)
    void onOpacityChanged(double val);
    void onParticleSizeChanged(double val);
    void onUseLogscaleChanged(bool checked);
    void onUseEmisiveChanged(bool checked);
    void onRangeMaxValueChanged(double val);
    void onRangeMinValueChanged(double val);
    void onCheckBoxAutoRangeChanged(bool checked);
    void onShowScalarBar(bool checked);
    void onNewCustomPaleteButtonClicked();
    void onEditPaleteButtonClicked();
    void onColorMapAdded(const Visualize::ColorMap& map);

  private:
    Ui::PropertyInspector* ui;
    QUuid                  m_currentNodeId;
    QSpace::Core::AppCore* m_app;
    void                   setupUiLogic();  // Начальное заполнение комбобоксов
    void                   updateWidgets(); // Синхронизация UI с данными ноды
};
} // namespace QSpace::UI