#pragma once
#include "Common/Interfaces/IRenderLayer.h"
#include "Common/Structures/CoreStructures.h"
#include "QSpaceScalarBar.h"
#include <cmath>
#include <iomanip>
#include <memory>
#include <sstream>
#include <vtkArrayCalculator.h>
#include <vtkAxis.h>
#include <vtkAxisActor2D.h>
#include <vtkColorTransferFunction.h>
#include <vtkContextScene.h>
#include <vtkContextView.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointGaussianMapper.h>
#include <vtkProp.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTextProperty.h>

namespace QSpace::Visualize {

class ParticleLayer : public IVtkRenderLayer {
  public:
    explicit ParticleLayer(std::shared_ptr<QSpace::Core::DataNode> node);
    void update() override;
    void attachInteractor(vtkRenderWindowInteractor* interactor) override;
    void detachInteractor() override;
    void swapData(std::shared_ptr<QSpace::Core::DataNode> node) override;
    void setVisible(bool visible) override;

    vtkSmartPointer<vtkProp> getVtkProp() override {
        return m_actor;
    };
    vtkSmartPointer<vtkScalarBarActor> getScalarBar() const override {
        return m_scalarBar;
    };
    bool isVisible() const override;
    void updateColorsForContrast(double contrast) override;

  private:
    std::shared_ptr<Core::DataNode>           m_node;
    vtkSmartPointer<vtkPointGaussianMapper>   m_mapper;
    vtkSmartPointer<vtkActor>                 m_actor;
    vtkSmartPointer<vtkColorTransferFunction> m_lut;
    vtkSmartPointer<vtkPiecewiseFunction>     m_opacityFunction;
    vtkSmartPointer<vtkScalarBarActor>        m_scalarBar;
    vtkSmartPointer<vtkScalarBarWidget>       m_scalarBarWidget;
    // vtkSmartPointer<vtkImageData>              m_imageData;
    // vtkSmartPointer<vtkGPUVolumeRayCastMapper> m_volumeMapper;
    // vtkSmartPointer<vtkVolume>                 m_volume;
    // vtkSmartPointer<vtkVolumeProperty>         m_volumeProperty;
    void applyColorMap(QUuid& colorMapUuid, double range[2]);
    void setupScalarBar();
    void setupDataArrays(vtkPolyData* data, Core::VisualSettings& s);
    void applyRenderModeSettings(const Core::VisualSettings& s);
    void updateShader(const Core::VisualSettings& s);
    void updateScalarBarVisibility(const Core::VisualSettings& s);
};

} // namespace QSpace::Visualize