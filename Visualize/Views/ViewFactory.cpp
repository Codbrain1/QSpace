#pragma once
#include "Common/Enums/ViewEnums.h"
#include "View3D/OpenGL3DWidget/OpenGL3DWidget.h"

namespace QSpace::Visualize::Views {
class AbstractView;

class ViewFactory {
  public:
    static std::shared_ptr<AbstractView> createView(ViewType type, QObject* parent = nullptr) {
        switch (type) {
            case ViewType::OpenGL3D:
                return std::make_shared<View3D::OpenGL3DWidget>(parent);
            default:
                return nullptr;
        };
    }
};
} // namespace QSpace::Visualize::Views