
#pragma once
#include <cmath>
#include <iomanip>
#include <sstream>
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h> // Обязательно добавьте этот инклюд
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>

namespace QSpace::Visualize {

class QSpaceScalarBar : public vtkScalarBarActor {
  public:
    static QSpaceScalarBar* New();
    vtkTypeMacro(QSpaceScalarBar, vtkScalarBarActor);

    // Включает режим обратного логарифмирования для подписей
    void SetUseInverseLogLabels(bool enable) {
        if (this->UseInverseLogLabels != enable) {
            this->UseInverseLogLabels = enable;
            this->Modified();
        }
    }

    // Переопределяем метод отрисовки, чтобы обновлять аннотации перед рендером
    int RenderOpaqueGeometry(vtkViewport* viewport) override {
        if (this->UseInverseLogLabels) {
            this->UpdateLogAnnotations();
        }
        return this->Superclass::RenderOpaqueGeometry(viewport);
    }

  protected:
    QSpaceScalarBar() {
        this->DrawTickLabelsOff(); // Выключаем стандартные "авто" цифры
        this->DrawAnnotationsOn(); // Включаем наши кастомные подписи
        this->AnnotationTextScalingOn();
    }

    void UpdateLogAnnotations() {
        vtkScalarsToColors* lut = this->GetLookupTable();
        if (!lut)
            return;

        // double range[2];
        auto range = lut->GetRange();

        // Очищаем старые аннотации
        vtkSmartPointer<vtkStringArray> annNames  = vtkSmartPointer<vtkStringArray>::New();
        vtkSmartPointer<vtkDoubleArray> annValues = vtkSmartPointer<vtkDoubleArray>::New();

        int numTicks = this->GetNumberOfLabels();
        for (int i = 0; i < numTicks; ++i) {
            // Линейно распределяем значения в лог-пространстве (например 0, 1, 2, 3...)
            double t      = (double)i / (numTicks - 1);
            double logVal = range[0] + t * (range[1] - range[0]);

            // Проводим операцию 10^x
            double physVal = std::pow(10.0, logVal);

            // Форматируем текст (научная нотация)
            std::stringstream ss;
            ss << std::scientific << std::setprecision(1) << physVal;

            annValues->InsertNextValue(logVal);
            annNames->InsertNextValue(ss.str());
        }

        lut->SetAnnotations(annValues, annNames);
    }

  private:
    bool UseInverseLogLabels = false;
};
} // namespace QSpace::Visualize