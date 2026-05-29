#include "StyleManager.h"

namespace QSpace::Core {

void StyleManager::syncLayerWithMaster(std::shared_ptr<Layer>    layer,
                                       std::shared_ptr<DataNode> node) {
    if (!layer || !node)
        return;
    layer->settings           = node->masterSettings;
    layer->isSyncedWithMaster = true;
    layer->update();
}

void StyleManager::makeLayerUnique(std::shared_ptr<Layer> layer) {
    if (!layer || !layer->isSyncedWithMaster)
        return;

    // Создаем глубокую копию текущих настроек
    layer->settings           = layer->settings->clone();
    layer->isSyncedWithMaster = false;
    // update() не вызываем, визуально ничего не изменилось
}

void StyleManager::copyStyle(std::shared_ptr<Layer>    source,
                             std::shared_ptr<Layer>    target,
                             std::shared_ptr<DataNode> targetNode) {
    if (!source || !target || !targetNode)
        return;

    // Если целевой слой был синхронизирован, мы должны сделать его уникальным перед применением
    // нового стиля, иначе мы перепишем мастер-настройки и изменим ВСЕ окна.
    if (target->isSyncedWithMaster) {
        makeLayerUnique(target);
    }

    target->settings->applyCompatible(*(source->settings), targetNode->type);
    target->update();
}

} // namespace QSpace::Core