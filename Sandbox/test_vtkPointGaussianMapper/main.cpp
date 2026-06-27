#include "Common/Enums/IOEnums.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Interfaces/IOFactory.h"
#include "Common/Interfaces/IView.h"
#include "Common/Interfaces/LayerFactory.h"
#include "Common/Structures/IOStructures.h"
#include "Visualize/RenderLayerSettings/ParticleLayer.h"
#include "Visualize/VtkView.h"
#include <QApplication>
#include <QDockWidget>
#include <QKeyEvent>
#include <QMainWindow>
#include <QString>
#include <qapplication.h>
#include <qcoreevent.h>
#include <qelapsedtimer.h>
#include <qlogging.h>
#include <qloggingcategory.h>
#include <qmainwindow.h>
#include <qnamespace.h>
#include "Structures/CoreStructures.h"
#include <iostream>
#include <memory>

class ShortcutHandler : public QObject {
  private:
    std::shared_ptr<QSpace::Visualize::ParticleLayer> m_layer;
    QSpace::Visualize::VtkView*                       m_view;
    QSpace::Core::VisualSettings*                     m_settings;

  public:
    ShortcutHandler(std::shared_ptr<QSpace::Visualize::ParticleLayer> layer,
                    QSpace::Visualize::VtkView*                       view,
                    QSpace::Core::VisualSettings*                     settings,
                    QObject*                                          parent = nullptr)
        : QObject(parent), m_layer(layer), m_view(view), m_settings(settings) {
    }

  protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_R) {
                qInfo() << "\n[Shortcut] 'R' pressed. Restarting particle rendering pipeline...";



                QElapsedTimer timer;
                timer.start();
                // 1. Принудительно заставляем VTK обновить конвейер данных (мапперы, шейдеры)
                m_layer->update();
                // 2. Вызываем перерисовку окна VTK
                m_view->renderForce();

                qInfo() << "[Shortcut] Hot Reload Rendering Time:" << timer.elapsed()
                        << "milliseconds\n";
                return true; // Событие обработано
            } else if (keyEvent->key() == Qt::Key_Up) {
                qInfo()
                    << "\n[Shortcut] 'Up arrow' pressed. Restarting particle rendering pipeline...";
                QElapsedTimer timer;
                timer.start();
                m_settings->opacity += 0.01;

                m_layer->update();
                // 2. Вызываем перерисовку окна VTK
                m_view->renderForce();
                qInfo() << "[Shortcut] Hot Reload Rendering Time:" << timer.elapsed()
                        << "milliseconds\n";
                return true; // Событие обработано

            } else if (keyEvent->key() == Qt::Key_Down) {
                qInfo() << "\n[Shortcut] 'Down arrow' pressed. Restarting particle rendering "
                           "pipeline...";
                QElapsedTimer timer;
                timer.start();
                m_settings->opacity -= 0.01;

                m_layer->update();
                // 2. Вызываем перерисовку окна VTK
                m_view->renderForce();
                qInfo() << "[Shortcut] Hot Reload Rendering Time:" << timer.elapsed()
                        << "milliseconds\n";
                return true; // Событие обработано
            }
        }
        return QObject::eventFilter(obj, event);
    }
};

int main(int argc, char* argv[]) {
    //  ----------- Читаем файл- ----------
    auto reader = QSpace::IO::IOFactory::createReader(QSpace::IO::FileFormat::BIN);
    reader->setPolicy(QSpace::IO::FilePolicy::ForceStandart);

    auto    scheme = QSpace::IO::SchemeFactory::createSheme_v2(QSpace::Visualize::EntityType::Gas,
                                                            QSpace::IO::FileFormat::BIN);
    QString path   = "D:/NIR/NIR_6_semestr/QSpace/Sandbox/TEST_DATA/TF250_v2/G_    0.bin";

    if (!reader) {
        qCritical() << "Exception create reader!";
        return 0;
    }

    // Чтение файла
    QElapsedTimer timer;
    timer.start();
    auto   readResult = reader->read(path, scheme);
    qint64 timestamp  = timer.elapsed();

    // Проверка результата чтения
    if (readResult.isSuccess()) {
        qInfo() << "read file: " << path.toStdString()
                << QString(" is Succses (Time = %1 miliseconds)\n").arg(timestamp);
    } else {
        qCritical() << "read file: " << path.toStdString()
                    << QString(" is not Succses (Time = %1 miliseconds)\n").arg(timestamp);
        return 0;
    }

    // -------- Создаем окно для отображения данных --------
    QApplication                 app(argc, argv);
    std::unique_ptr<QMainWindow> window = std::make_unique<QMainWindow>();
    window->setFixedSize(900, 700);
    window->show();

    auto view = std::make_unique<QSpace::Visualize::VtkView>();

    if (!view) {
        qCritical() << "view is not Created!";
        return 0;
    }

    auto viewWidget = view->getWidget();
    window->setCentralWidget(viewWidget);


    // ----------- Добавляем данные на сцену ---------
    // создаем запись данных
    auto                                    timeStart = timer.elapsed();
    std::shared_ptr<QSpace::Core::DataNode> node =
        std::make_shared<QSpace::Core::DataNode>(readResult.data,
                                                 "test gas",
                                                 readResult.timestamp,
                                                 QSpace::Visualize::EntityType::Gas);
    timestamp = timer.elapsed();
    qInfo() << "Time DataNode created: " << timestamp - timeStart << " milliseconds";

    if (!node) {
        qCritical() << "node is not created!!";
        return 0;
    }
    // node->masterSettings->useLogScale = false;
    // создаем слой с настройками
    timeStart = timer.elapsed();

    auto layer = std::make_shared<QSpace::Visualize::ParticleLayer>(node);
    layer->setSettings(node->masterSettings);
    layer->setData(node);
    view->addProp(layer->getVtkProp());

    if (auto interactor = view->getInteractor()) {
        layer->attachInteractor(interactor);
    }

    timestamp = timer.elapsed();
    qInfo() << "Time Layer created: " << timestamp - timeStart << " milliseconds";

    // обновляем сцену
    timeStart = timer.elapsed();
    // Запускаем обновление пайплайна VTK (чтение массивов, передача в маппер)
    // layer->update();
    // view->render();
    timestamp = timer.elapsed();
    qInfo() << "Time Data Rendering: " << timestamp - timeStart << " milliseconds";


    ShortcutHandler* handler =
        new ShortcutHandler(layer, view.get(), node->masterSettings.get(), window.get());
    window->installEventFilter(handler);
    // Дополнительно вешаем на сам виджет VTK, так как при фокусе на сцену окно может не перехватить
    // нажатие
    if (viewWidget) {
        viewWidget->installEventFilter(handler);
    }
    return app.exec();
}