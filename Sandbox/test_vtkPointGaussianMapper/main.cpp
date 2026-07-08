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
    std::shared_ptr<QSpace::Visualize::ParticleLayer> m_layer1;
    std::shared_ptr<QSpace::Visualize::ParticleLayer> m_layer2;
    std::shared_ptr<QSpace::Visualize::ParticleLayer> m_layer3;
    std::shared_ptr<QSpace::Visualize::ParticleLayer> m_layer4;
    std::shared_ptr<QSpace::Visualize::ParticleLayer> m_layer5;

    QSpace::Visualize::VtkView*               m_view;
    QSpace::Visualize::Layers::LayerSettings* m_settings;

  public:
    ShortcutHandler(std::shared_ptr<QSpace::Visualize::ParticleLayer> layer,
                    std::shared_ptr<QSpace::Visualize::ParticleLayer> layer1,
                    std::shared_ptr<QSpace::Visualize::ParticleLayer> layer2,
                    std::shared_ptr<QSpace::Visualize::ParticleLayer> layer3,
                    std::shared_ptr<QSpace::Visualize::ParticleLayer> layer4,
                    std::shared_ptr<QSpace::Visualize::ParticleLayer> layer5,
                    QSpace::Visualize::VtkView*                       view,
                    QSpace::Visualize::Layers::LayerSettings*         settings,
                    QObject*                                          parent = nullptr)
        : QObject(parent),
          m_layer(layer),
          m_layer1(layer1),
          m_layer2(layer2),
          m_layer3(layer3),
          m_layer4(layer4),
          m_layer5(layer5),
          m_view(view),
          m_settings(settings) {
    }

  protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_R) {
                qInfo() << "\n[Shortcut] 'R' pressed. Restarting particle rendering pipeline...";



                // 1. Принудительно заставляем VTK обновить конвейер данных (мапперы, шейдеры)
                QElapsedTimer timer;
                timer.start();
                m_layer->update();
                m_layer1->update();
                m_layer2->update();
                m_layer3->update();
                m_layer4->update();
                m_layer5->update();
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

    auto scheme = QSpace::IO::SchemeFactory::createSheme_v2(QSpace::Visualize::EntityType::Gas,
                                                            QSpace::IO::FileFormat::BIN);
    auto scheme1 =
        QSpace::IO::SchemeFactory::createSheme_v2(QSpace::Visualize::EntityType::DarkMatter,
                                                  QSpace::IO::FileFormat::BIN);
    QString path  = "D:/NIR/NIR_6_semestr/QSpace/Sandbox/TEST_DATA/TF250_v2/G_    0.bin";
    QString path1 = "D:/NIR/NIR_6_semestr/QSpace/Sandbox/TEST_DATA/TF250_v2/DM_    0.bin";
    QString path2 = "D:/NIR/NIR_6_semestr/QSpace/Sandbox/TEST_DATA/TF250_v2/DM_    1.bin";
    QString path3 = "D:/NIR/NIR_6_semestr/QSpace/Sandbox/TEST_DATA/TF250_v2/DM_    2.bin";
    QString path4 = "D:/NIR/NIR_6_semestr/QSpace/Sandbox/TEST_DATA/TF250_v2/DM_    3.bin";
    QString path5 = "D:/NIR/NIR_6_semestr/QSpace/Sandbox/TEST_DATA/TF250_v2/DM_    4.bin";

    if (!reader) {
        qCritical() << "Exception create reader!";
        return 0;
    }

    // Чтение файла
    QElapsedTimer timer;
    timer.start();
    auto   readResult  = reader->read(path, scheme);
    auto   readResult1 = reader->read(path1, scheme1);
    auto   readResult2 = reader->read(path2, scheme1);
    auto   readResult3 = reader->read(path3, scheme1);
    auto   readResult4 = reader->read(path4, scheme1);
    auto   readResult5 = reader->read(path5, scheme1);
    qint64 timestamp   = timer.elapsed();

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
    std::shared_ptr<QSpace::Core::DataNode> node1 =
        std::make_shared<QSpace::Core::DataNode>(readResult1.data,
                                                 "test DM0",
                                                 readResult1.timestamp,
                                                 QSpace::Visualize::EntityType::DarkMatter);
    std::shared_ptr<QSpace::Core::DataNode> node2 =
        std::make_shared<QSpace::Core::DataNode>(readResult2.data,
                                                 "test DM1",
                                                 readResult2.timestamp,
                                                 QSpace::Visualize::EntityType::DarkMatter);
    std::shared_ptr<QSpace::Core::DataNode> node3 =
        std::make_shared<QSpace::Core::DataNode>(readResult3.data,
                                                 "test DM2",
                                                 readResult3.timestamp,
                                                 QSpace::Visualize::EntityType::DarkMatter);
    std::shared_ptr<QSpace::Core::DataNode> node4 =
        std::make_shared<QSpace::Core::DataNode>(readResult4.data,
                                                 "test DM3",
                                                 readResult4.timestamp,
                                                 QSpace::Visualize::EntityType::DarkMatter);
    std::shared_ptr<QSpace::Core::DataNode> node5 =
        std::make_shared<QSpace::Core::DataNode>(readResult5.data,
                                                 "test DM4",
                                                 readResult5.timestamp,
                                                 QSpace::Visualize::EntityType::DarkMatter);

    timestamp = timer.elapsed();
    qInfo() << "Time DataNode created: " << timestamp - timeStart << " milliseconds";

    if (!node) {
        qCritical() << "node is not created!!";
        return 0;
    }
    node->masterSettings->useLogScale  = false;
    node1->masterSettings->useLogScale = false;
    node2->masterSettings->useLogScale = false;
    node3->masterSettings->useLogScale = false;
    node4->masterSettings->useLogScale = false;
    node5->masterSettings->useLogScale = false;
    // создаем слой с настройками
    timeStart = timer.elapsed();

    auto layer  = std::make_shared<QSpace::Visualize::ParticleLayer>(node);
    auto layer1 = std::make_shared<QSpace::Visualize::ParticleLayer>(node1);
    auto layer2 = std::make_shared<QSpace::Visualize::ParticleLayer>(node2);
    auto layer3 = std::make_shared<QSpace::Visualize::ParticleLayer>(node3);
    auto layer4 = std::make_shared<QSpace::Visualize::ParticleLayer>(node4);
    auto layer5 = std::make_shared<QSpace::Visualize::ParticleLayer>(node5);
    layer->setSettings(node->masterSettings);
    layer->setData(node);

    layer1->setSettings(node1->masterSettings);
    layer1->setData(node1);

    layer2->setSettings(node2->masterSettings);
    layer2->setData(node2);

    layer3->setSettings(node3->masterSettings);
    layer3->setData(node3);

    layer4->setSettings(node4->masterSettings);
    layer4->setData(node4);

    layer5->setSettings(node5->masterSettings);
    layer5->setData(node5);

    view->addProp(layer->getVtkProp());
    view->addProp(layer1->getVtkProp());
    view->addProp(layer2->getVtkProp());
    view->addProp(layer3->getVtkProp());
    view->addProp(layer4->getVtkProp());
    view->addProp(layer5->getVtkProp());

    if (auto interactor = view->getInteractor()) {
        layer->attachInteractor(interactor);
        layer1->attachInteractor(interactor);
        layer2->attachInteractor(interactor);
        layer3->attachInteractor(interactor);
        layer4->attachInteractor(interactor);
        layer5->attachInteractor(interactor);
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


    ShortcutHandler* handler = new ShortcutHandler(layer,
                                                   layer1,
                                                   layer2,
                                                   layer3,
                                                   layer4,
                                                   layer5,
                                                   view.get(),
                                                   node->masterSettings.get(),
                                                   window.get());
    window->installEventFilter(handler);
    // Дополнительно вешаем на сам виджет VTK, так как при фокусе на сцену окно может не перехватить
    // нажатие
    if (viewWidget) {
        viewWidget->installEventFilter(handler);
    }
    return app.exec();
}