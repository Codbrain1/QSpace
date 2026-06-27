#pragma once
#include "Common/Interfaces/IRenderLayer.h"
#include "Common/Structures/CoreStructures.h"
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
#include "BaseLayer.h"
#include "QSpaceScalarBar.h"
#include <cmath>
#include <iomanip>
#include <memory>
#include <sstream>


namespace QSpace::Visualize {

class ParticleLayer : public BaseLayer {
  public:
    explicit ParticleLayer(std::shared_ptr<QSpace::Core::DataNode> node);

    vtkSmartPointer<vtkProp> getVtkProp() override {
        return m_actor;
    };

  private:
    vtkSmartPointer<vtkPointGaussianMapper> m_mapper;
    vtkSmartPointer<vtkActor>               m_actor;
    void                                    setupDataArrays(Core::VisualSettings& s) override final;
    void applyRenderModeSettings(const Core::VisualSettings& s) override final;
    void updateShader(const Core::VisualSettings& s);

    void postUpdate(const Core::VisualSettings& s) override final {
        if (s.mode == RenderMode::GausianSplat) {
            updateShader(s);
        }
    }
};

} // namespace QSpace::Visualize