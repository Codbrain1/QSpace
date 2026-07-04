#pragma once
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSmartPointer.h>

class vtkPointPicker;

/**
 * Специализированный стиль навигации для облаков частиц галактик.
 */
class GalacticInteractorStyle : public vtkInteractorStyleTrackballCamera {
  public:
    static GalacticInteractorStyle* New();
    vtkTypeMacro(GalacticInteractorStyle, vtkInteractorStyleTrackballCamera);

    void OnChar() override;

  protected:
    GalacticInteractorStyle();
    void PerformGalacticFocus();

  private:
    vtkSmartPointer<vtkPointPicker> PointPicker;
};