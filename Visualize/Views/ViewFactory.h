#pragma once
#include "Common/Enums/ViewEnums.h"

namespace QSpace::Visualize::Views {
class AbstractView;

namespace ViewFactory {

std::shared_ptr<AbstractView> createView(ViewType type, QObject* parent = nullptr);

}; // namespace ViewFactory
} // namespace QSpace::Visualize::Views