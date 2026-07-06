#include "Visualize/Views/View3D/OpenGL3DWidget/OpenGL3DWidget.h"
#include "Visualize/Views/ViewFactory.h"
#include <QApplication>
#include <QWindow>
#include <qmainwindow.h>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    auto         w = QSpace::Visualize::Views::ViewFactory::createView(
        QSpace::Visualize::Views::ViewType::OpenGL3D);
    std::unique_ptr<QMainWindow> window = std::make_unique<QMainWindow>();
    window->setCentralWidget(w.get()->getWidget());
    window->show();
    return app.exec();
}