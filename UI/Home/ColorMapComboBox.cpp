#include "ColorMapComboBox.h"
#include <QAbstractItemView>
#include <QFrame>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qwidget.h>

namespace QSpace::UI {
ColorMapComboBox::ColorMapComboBox(QWidget* parent) : QComboBox(parent) {
}
void ColorMapComboBox::showPopup() {
    QComboBox::showPopup();
    QWidget* popup = this->view()->window(); // получение контейнера выпадающего окна
    if (!m_footerButton) {
        // настраиваем выравнивание на дне виджета
        QBoxLayout* existingLayout = qobject_cast<QBoxLayout*>(popup->layout());
        if (existingLayout) {
            // рисуем линию для отделения кнопки от пунктов меню
            QFrame* line = new QFrame(popup);
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);

            m_footerButton = new QPushButton("Custom", popup); // создаем кнопку
            m_footerButton->setFlat(true);                     // убирает рамки
            m_footerButton->setContentsMargins(5, 5, 5, 5);
            existingLayout->addWidget(this->view());
            existingLayout->addWidget(line);
            existingLayout->addWidget(m_footerButton);

            popup->setLayout(existingLayout);
            connect(m_footerButton, &QPushButton::clicked, this, [this]() {
                this->hidePopup();
                emit customButtonClicked();
            });
        }
    }
}
} // namespace QSpace::UI