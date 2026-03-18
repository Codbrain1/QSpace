#pragma once

#include "Structures/RenderStructures.h"
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class ColorMapEditorDialog;
}
QT_END_NAMESPACE

namespace QSpace::UI {
class ColorMapEditorDialog : public QDialog {
    Q_OBJECT
  public:
    explicit ColorMapEditorDialog(const QSpace::Visualize::ColorMap& baseMap, QWidget* parent = nullptr);
    ~ColorMapEditorDialog();
    Visualize::ColorMap getEditedMap() const;

  private:
    Ui::ColorMapEditorDialog* ui; // Теперь это совпадет с именем в .ui
    Visualize::ColorMap       m_baseMap;
    void                      populateTable();
};
} // namespace QSpace::UI