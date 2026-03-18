#include "ColorMapEditorDialog.h"
#include "ui_ColorMapEditorDialog.h"
#include <QColorDialog>
#include <QTableWidgetItem>
#include <QUuid>


namespace QSpace::UI {

ColorMapEditorDialog::ColorMapEditorDialog(const Visualize::ColorMap& baseMap, QWidget* parent)
    : QDialog(parent), ui(new Ui::ColorMapEditorDialog), m_baseMap(baseMap) {
    ui->setupUi(this);

    // Связываем стандартные кнопки Ok и Cancel с закрытием диалога
    // QDialog::accept вернет QDialog::Accepted при вызове exec()
    // QDialog::reject вернет QDialog::Rejected
    connect(ui->pushButtonOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &QDialog::reject);

    // Разрешаем редактировать таблицу только по двойному клику
    ui->tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked);

    // Обработка двойного клика для изменения цвета
    connect(ui->tableWidget, &QTableWidget::cellDoubleClicked, this, [this](int row, int column) {
        if (column == 1) { // Кликнули по колонке с цветом
            QColor currentColor = ui->tableWidget->item(row, column)->background().color();
            QColor newColor     = QColorDialog::getColor(currentColor, this, "Выберите цвет");

            if (newColor.isValid()) {
                ui->tableWidget->item(row, column)->setBackground(newColor);
            }
        }
    });

    populateTable();
}

ColorMapEditorDialog::~ColorMapEditorDialog() {
    delete ui;
}

void ColorMapEditorDialog::populateTable() {
    ui->tableWidget->setRowCount(0);

    for (const auto& pt : m_baseMap.points) {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        // Уровень (число)
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(pt.x, 'f', 4)));

        // Цвет (фон ячейки)
        auto* colorItem = new QTableWidgetItem();
        colorItem->setBackground(QColor::fromRgbF(pt.r, pt.g, pt.b));
        // Запрещаем ввод текста в ячейку с цветом
        colorItem->setFlags(colorItem->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(row, 1, colorItem);
    }
}

Visualize::ColorMap ColorMapEditorDialog::getEditedMap() const {
    Visualize::ColorMap newMap;
    // Генерируем уникальный ID для новой палитры
    newMap.id       = QUuid::createUuid();
    newMap.name     = "Custom_" + newMap.id.toString().mid(1, 4);
    newMap.isPreset = false;

    // Собираем данные из таблицы
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        double x = ui->tableWidget->item(i, 0)->text().toDouble();
        QColor c = ui->tableWidget->item(i, 1)->background().color();
        newMap.points.push_back({x, c.redF(), c.greenF(), c.blueF()});
    }

    // Сортируем по 'x', так как для рендера точки должны идти по возрастанию
    std::sort(newMap.points.begin(), newMap.points.end(), [](const auto& a, const auto& b) { return a.x < b.x; });

    return newMap;
}

} // namespace QSpace::UI