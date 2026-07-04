#include "QSpaceScalarBar.h"

#include <cmath>
#include <vtkCoordinate.h>
#include <vtkDoubleArray.h>
#include <vtkObjectFactory.h>
#include <vtkScalarsToColors.h>
#include <vtkTextProperty.h>
#include <vtkViewport.h>

namespace QSpace::Visualize {

vtkStandardNewMacro(QSpaceScalarBar);

QSpaceScalarBar::QSpaceScalarBar() {
    this->LogMode = false;

    // Инициализируем массив для кастомных меток
    this->CustomLabelsArray = vtkSmartPointer<vtkDoubleArray>::New();
}

void QSpaceScalarBar::SetLogMode(bool enabled) {
    if (this->LogMode == enabled)
        return;
    this->LogMode = enabled;
    this->Modified(); // Здесь Modified() уместен, так как это сеттер, вызываемый извне
}

int QSpaceScalarBar::RenderOpaqueGeometry(vtkViewport* viewport) {
    // Подготавливаем метки до начала отрисовки
    PrepareCustomLabels();
    return vtkScalarBarActor::RenderOpaqueGeometry(viewport);
}

int QSpaceScalarBar::RenderOverlay(vtkViewport* viewport) {
    return vtkScalarBarActor::RenderOverlay(viewport);
}
void QSpaceScalarBar::PrepareCustomLabels() {
    vtkScalarsToColors* lut = this->GetLookupTable();
    if (!lut || !lut->GetRange())
        return;

    // ВАЖНО: Очищаем старые аннотации перед любыми действиями

    if (!this->LogMode) {
        // Возвращаем стандартный вид
        this->SetUseCustomLabels(false);
        this->SetDrawTickLabels(true);
        this->SetDrawAnnotations(false);
        this->SetLabelFormat("%g");
        return;
    }

    // Для логарифмического режима: ОТКЛЮЧАЕМ стандартные метки и ВКЛЮЧАЕМ аннотации
    this->SetUseCustomLabels(false);
    this->SetDrawTickLabels(false);
    this->SetDrawAnnotations(true);

    double* range  = lut->GetRange();
    double  minLog = range[0];
    double  maxLog = range[1];

    int numLabels = 8; // Строго 8 меток

    if (minLog < maxLog) {
        double step = (maxLog - minLog) / static_cast<double>(numLabels - 1);

        for (int i = 0; i < numLabels; ++i) {
            lut->RemoveAnnotation(i);
            // 1. Вычисляем ПОЗИЦИЮ на шкале (в логарифмах: -2, -1.5, -1...)
            double logPos = minLog + (step * i);

            // 2. Вычисляем ФИЗИЧЕСКОЕ ЗНАЧЕНИЕ для текста (0.01, 0.03, 0.1...)
            double physValue = std::pow(10.0, logPos);

            // 3. Форматируем текст в научный вид
            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), "%.1e", physValue);

            // 4. Добавляем аннотацию: позиция -> текст
            lut->SetAnnotation(logPos, buffer);
        }
    } else {
        // Защита от нулевого диапазона
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%.1e", std::pow(10.0, minLog));
        lut->SetAnnotation(minLog, buffer);
    }

    // Настраиваем шрифт аннотаций, чтобы они выглядели как обычные цифры шкалы
    if (this->GetAnnotationTextProperty()) {
        // Копируем стиль из основного текста меток (цвет, шрифт, размер)
        if (this->GetLabelTextProperty()) {
            this->GetAnnotationTextProperty()->ShallowCopy(this->GetLabelTextProperty());
        }
        this->GetAnnotationTextProperty()->SetJustificationToLeft();
        this->GetAnnotationTextProperty()->SetVerticalJustificationToCentered();
    }
}

} // namespace QSpace::Visualize