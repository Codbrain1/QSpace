#include "IView.h"
#include <qobject.h>
namespace QSpace::Visualize
{
IView::IView(QObject *parent)
    : QObject(parent)
{
}
} // namespace QSpace::Visualize