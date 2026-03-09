#pragma once
#include "Common/Interfaces/IRenderLayer.h"
#include "Common/Structures/CoreStructures.h"
#include <memory>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointGaussianMapper.h>
#include <vtkProp.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>

namespace QSpace::Visualize {

class ParticleLayer : public IRenderLayer {
  public:
    explicit ParticleLayer(std::shared_ptr<QSpace::Core::DataNode> node);
    void                     update() override;
    vtkSmartPointer<vtkProp> getVtkProp() override {
        return m_actor;
    };
    vtkSmartPointer<vtkScalarBarActor> getScalarBar() const override {
        return m_scalarBar;
    };
    void swapData(std::shared_ptr<QSpace::Core::DataNode> node) override;

  private:
    std::shared_ptr<Core::DataNode>           m_node;
    vtkSmartPointer<vtkPointGaussianMapper>   m_mapper;
    vtkSmartPointer<vtkActor>                 m_actor;
    vtkSmartPointer<vtkColorTransferFunction> m_lut;
    vtkSmartPointer<vtkPiecewiseFunction>     m_opacityFunction;
    vtkSmartPointer<vtkScalarBarActor>        m_scalarBar;
    void                                      applyColorMap(QSpace::Visualize::ColorMapType type, double range[2]);
    void                                      setupScalarBar();
};
} // namespace QSpace::Visualize