#pragma once
#include <vtkAxisActor2D.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarsToColors.h>
#include <vtkSmartPointer.h>
#include <vtkViewport.h>

namespace QSpace::Visualize {

class QSpaceScalarBar : public vtkScalarBarActor {
  public:
    vtkTypeMacro(QSpaceScalarBar, vtkScalarBarActor);
    static QSpaceScalarBar* New();

    // Установка режима логарифмической шкалы
    void SetLogMode(bool enabled);
    bool GetLogMode() const {
        return this->LogMode;
    }

    int RenderOverlay(vtkViewport* viewport) override;
    int RenderOpaqueGeometry(vtkViewport* viewport) override;

  protected:
    QSpaceScalarBar();
    ~QSpaceScalarBar() override = default;

    bool                            LogMode;
    vtkSmartPointer<vtkDoubleArray> CustomLabelsArray;

  private:
    void PrepareCustomLabels();

    // Запрещаем копирование
    QSpaceScalarBar(const QSpaceScalarBar&) = delete;
    void operator=(const QSpaceScalarBar&)  = delete;
};

} // namespace QSpace::Visualize