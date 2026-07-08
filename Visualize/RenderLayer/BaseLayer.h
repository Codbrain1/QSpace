#pragma once
#include "Common/Interfaces/IRenderLayer.h"
#include <vtkAbstractMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkSmartPointer.h>
#include <vtkSmartPointerBase.h>
#include "QSpaceScalarBar.h"

namespace QSpace::Visualize {
class BaseLayer : public IVtkRenderLayer {
  public:
    void attachInteractor(vtkRenderWindowInteractor* interactor) override;
    void detachInteractor() override;
    void updateColorsForContrast(double contrast) override;

    vtkSmartPointer<vtkScalarBarActor> getScalarBar() const override {
        return m_scalarBar;
    };

    void update() override;
    void setData(std::weak_ptr<QSpace::Core::DataNode> node) override;
    void setSettings(std::shared_ptr<QSpace::Visualize::Layers::LayerSettings> settings) override;
    void setVisible(bool visible) override;
    bool isVisible() const override;

  protected:
    BaseLayer(std::shared_ptr<QSpace::Core::DataNode> node);
    virtual ~BaseLayer() = default;
    vtkSmartPointer<vtkColorTransferFunction> m_lut;             // для цветовой карты
    vtkSmartPointer<vtkPiecewiseFunction>     m_opacityFunction; // для прозрачности
    vtkSmartPointer<QSpaceScalarBar>          m_scalarBar;       // для легенды
    vtkSmartPointer<vtkScalarBarWidget> m_scalarBarWidget; // для управления отображением легенды

    std::weak_ptr<Core::DataNode>                     m_node; // данные
    std::shared_ptr<Visualize::Layers::LayerSettings> m_settings;

    vtkSmartPointer<vtkProp>           m_baseProp;
    vtkSmartPointer<vtkAbstractMapper> m_baseMapper;

    // ----- setup -----
    void setupLayer();
    void setupScalarBar();
    // ----- update -----
    void         applyColorMap(const QUuid& colorMapUuid, double range[2]);
    void         updateScalarBarVisibility(const Visualize::Layers::LayerSettings& s);
    virtual void setupDataArrays(Visualize::Layers::LayerSettings& s)               = 0;
    virtual void applyRenderModeSettings(const Visualize::Layers::LayerSettings& s) = 0;

    virtual void postUpdate(const Visualize::Layers::LayerSettings& s) {
    }
};
} // namespace QSpace::Visualize