#include "QSpaceScalarBar.h"

namespace QSpace::Visualize {

// Этот макрос генерирует реализацию метода New()
// и позволяет VTK корректно инстанцировать ваш класс
vtkStandardNewMacro(QSpaceScalarBar);

// Если вы оставили методы UpdateLogAnnotations и RenderOpaqueGeometry
// внутри хидера как inline, то здесь больше ничего не нужно.
// Но лучше вынести их сюда для чистоты сборки.

} // namespace QSpace::Visualize