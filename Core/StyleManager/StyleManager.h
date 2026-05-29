#pragma once
#include "Core/LayerManager/LayerManager.h"
#include <memory>

namespace QSpace::Core {

class StyleManager {
  public:
    // Сценарий 1: Привязать слой к общим настройкам узла (Master)
    static void syncLayerWithMaster(std::shared_ptr<Layer> layer, std::shared_ptr<DataNode> node);

    // Сценарий 2: Сделать настройки слоя уникальными (отвязать)
    static void makeLayerUnique(std::shared_ptr<Layer> layer);

    // Сценарий 4: Скопировать стиль с одного слоя на другой
    static void copyStyle(std::shared_ptr<Layer>    source,
                          std::shared_ptr<Layer>    target,
                          std::shared_ptr<DataNode> targetNode);
};

} // namespace QSpace::Core