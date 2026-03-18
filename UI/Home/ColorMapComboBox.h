#pragma once
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <qcombobox.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qtmetamacros.h>
#include <qwidget.h>

namespace QSpace::UI {
class ColorMapComboBox : public QComboBox {
    Q_OBJECT
  public:
    explicit ColorMapComboBox(QWidget* parent = nullptr);
  signals:
    void customButtonClicked();

  protected:
    void showPopup() override;

  private:
    QPushButton* m_footerButton = nullptr;
};
} // namespace QSpace::UI