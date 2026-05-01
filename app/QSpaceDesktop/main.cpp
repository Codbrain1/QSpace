#include "Core/AppCore/AppCore.h"
#include "Core/LogManager/LogManager.h"
#include <QApplication> //заголовок включающий основной графический класс приложения
#include <QApplication>
#include <QLocale> //заголовок включающий класс конвертации чисел и их строковых предсталений на различные языки
#include <QMainWindow>
#include <QPushButton>
#include <QTranslator> //заголовок класса для перевода на различные языки
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>
#include <QtGlobal>
#include <qloggingcategory.h>
#include <qmainwindow.h>
#include <vtkAutoInit.h>
#include <vtkGaussianSplatter.h>
#include <vtkInteractionStyleModule.h>
#include <vtkOutputWindow.h>
#include <vtkRenderingOpenGL2Module.h>
#include <vtkRenderingVolumeOpenGL2Module.h>
#include <vtkType.h>
#include "Logger/Logger.h"
#include "UI/Home/MainWindow.h"

int main(int argc, char* argv[]) {
    vtkOutputWindow::SetGlobalWarningDisplay(0);
    QApplication app(argc, argv); // создание экземпляра приложения
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();

    fmt.setAlphaBufferSize(16);
    fmt.setSamples(0);

    QSurfaceFormat::setDefaultFormat(fmt);

    QSpace::Core::LogManager::setup(); // инициализируем запись логов
    QSpace::Core::AppCore core;

    QTranslator       translator; // создание экземпляра переводчика на разные языки
    const QStringList uiLanguages =
        QLocale::system().uiLanguages(); // вытаскиваем доступные в системе языки
    qCInfo(LogSystem) << "Initialize Translations...";
    bool isTanslatesLoaded = false;

    for (const QString& locale : uiLanguages) {
        QLocale loc(locale);
        if (translator.load(loc, "QSpace", "_", ":/i18n")) {
            app.installTranslator(&translator);
            qCInfo(LogSystem) << "Lainguage loaded:" << loc.name();
            isTanslatesLoaded = true;
            break;
        }
    }
    if (!isTanslatesLoaded)
        qCWarning(LogSystem, "No suitable translation found. Falling back to defult (English)");

    qCInfo(LogSystem) << "Application (QSpace) Starting...";
    qCDebug(LogSystem) << "Built with Qt version:" << QT_VERSION_STR;
    qCDebug(LogSystem) << "Running with Qt version:" << qVersion();
    QSpace::UI::MainWindow window(&core); // создание экземпляра окна
    window.show();                        // отображение окна
    return app.exec();
}
