#include "IView.h"
#include <qobject.h>
namespace QSpace::Visualize
{
IView::IView(QObject *parent)
    : QObject(parent)
{
}
IView3D::IView3D(QObject *parent)
    : IView(parent)
{
}
} // namespace QSpace::Visualize