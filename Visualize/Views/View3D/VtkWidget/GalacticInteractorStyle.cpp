#include "GalacticInteractorStyle.h"
#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkPointPicker.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(GalacticInteractorStyle);

GalacticInteractorStyle::GalacticInteractorStyle() {
    this->PointPicker = vtkSmartPointer<vtkPointPicker>::New();
    // Допуск 1% от диагонали окна для захвата точек-частиц [6]
    this->PointPicker->SetTolerance(0.01);
}

void GalacticInteractorStyle::OnChar() {
    vtkRenderWindowInteractor* rwi = this->Interactor;
    if (!rwi)
        return;

    char key = rwi->GetKeyCode();

    // Исправлено условие: корректное логическое ИЛИ
    if (key == 'f' || key == 'F') {
        this->PerformGalacticFocus();
    } else {
        this->Superclass::OnChar();
    }
}

void GalacticInteractorStyle::PerformGalacticFocus() {
    if (!this->CurrentRenderer)
        return;

    // 1. Получаем позицию курсора (x, y в пикселях)
    int* eventPos = this->Interactor->GetEventPosition();

    // 2. Выполняем выбор. Передаем именно элементы массива
    if (this->PointPicker->Pick(static_cast<double>(eventPos[0]),
                                static_cast<double>(eventPos[1]),
                                0.0,
                                this->CurrentRenderer)) {
        // 3. Получаем мировые координаты (нужен массив строго на 3 элемента) [4]
        double pickedCoords[3];
        this->PointPicker->GetPickPosition(pickedCoords);

        // 4. Устанавливаем новую точку вращения камеры
        vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
        // Используем индексы 0, 1, 2
        camera->SetFocalPoint(pickedCoords[0], pickedCoords[1], pickedCoords[2]);

        // Предотвращаем "заваливание" горизонта при вращении
        camera->OrthogonalizeViewUp();

        // 5. Обязательно вызываем Render для обновления окна
        this->Interactor->Render();
    }
}