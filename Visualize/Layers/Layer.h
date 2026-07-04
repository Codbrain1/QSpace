#pragma once

struct Layer {
    QUuid                                    layerId;
    QUuid                                    dataNodeId;
    QString                                  name;
    std::weak_ptr<DataNode>                  dataNode;
    std::weak_ptr<Visualize::IView>          view;
    std::shared_ptr<VisualSettings>          settings;
    std::shared_ptr<Visualize::IRenderLayer> renderEngine;
    bool                                     isSyncedWithMaster = true;

    Layer(std::shared_ptr<DataNode> node, std::shared_ptr<Visualize::IView> targetView)
        : layerId(QUuid::createUuid()),
          dataNodeId(node->id),
          dataNode(node),
          view(targetView),
          settings(node->masterSettings) {
    }

    // Метод для связки после создания renderEngine
    void assignEngine(std::shared_ptr<Visualize::IRenderLayer> engine) {
        renderEngine = engine;
        if (renderEngine) {
            renderEngine->setSettings(settings);
            renderEngine->setData(dataNode);
        }
    }

    void setVisible(bool visible) {
        settings->isVisible = visible;
        renderEngine->setVisible(visible);
    }

    void update() {
        auto v = view.lock();
        auto d = dataNode.lock();
        if (v && d && renderEngine) {
            renderEngine->update();
        }
    }
};