#pragma once
#include "Common/Enums/ViewEnums.h"

namespace QSpace::Visualize::Views {
class AbstractView;

class ViewFactory {
  public:
    static std::shared_ptr<AbstractView> createView(ViewType type, QObject* parent = nullptr);
};
} // namespace QSpace::Visualize::Views