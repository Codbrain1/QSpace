#pragma once
#include <QMap>
#include <QWidget>

class QFormLayout;
class QMetaProperty;
class QPushButton;
class QColor;

namespace QSpace::Visualize::Layers {
class LayerSettings;
}
namespace QSpace::UI {
class SettingsEditorWidget : public QWidget {
    Q_OBJECT
  public:
    explicit SettingsEditorWidget(QWidget* parent = nullptr);
    ~SettingsEditorWidget() = default;

    // Назначает текущий объект настроек и полностью перестраивает UI формы
    void setSettings(QSpace::Visualize::Layers::LayerSettings* settings);

  private:
    void createRowForProperty(const QMetaProperty& prop);
    void updateColorButton(QPushButton* btn, const QColor& color);
    void updateUiValues();
    void clearEditor();

    QFormLayout*                              m_layout   = nullptr;
    QSpace::Visualize::Layers::LayerSettings* m_settings = nullptr;
    QMap<QString, QWidget*>                   m_editors;
};
} // namespace QSpace::UI