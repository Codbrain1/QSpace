#pragma once
#include "Common/Structures/CoreStructures.h"
#include <vtkProp.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>

namespace QSpace::Visualize
{
class IRenderLayer
{
public:
  virtual ~IRenderLayer() = default;
  virtual void update() = 0;
  virtual vtkSmartPointer<vtkProp> getVtkProp() = 0;
  virtual void swapData(std::shared_ptr<QSpace::Core::DataNode> node) = 0;
  virtual vtkSmartPointer<vtkScalarBarActor> getScalarBar() const
  {
    return nullptr;
  };
};
} // namespace QSpace::Visualize