#include "Core/LogManager/LogManager.h"
#include "Logger/Logger.h"
#include <QApplication> //заголовок включающий основной графический класс приложения
#include <QLocale>      //заголовок включающий класс конвертации чисел и их строковых предсталений на различные языки
#include <QTranslator>  //заголовок класса для перевода на различные языки
#include <qloggingcategory.h>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv); // создание экземпляра приложения

    QSpace::Core::LogManager::setup(); // инициализируем запись логов

    qCInfo(LogSystem) << "Application (QSpace) Starting...";

    QTranslator       translator;                                    // создание экземпляра переводчика на разные языки
    const QStringList uiLanguages = QLocale::system().uiLanguages(); // вытаскиваем доступные в системе языки
    qCInfo(LogSystem) << "Initialize Translations...";
    bool isTanslatesLoaded = false;

    for (const QString& locale : uiLanguages) {
        QLocale loc(locale);
        if (translator.load(loc, "QSpace", "_", ":/i18n")) {
            a.installTranslator(&translator);
            qCInfo(LogSystem) << "Lainguage loaded:" << loc.name();
            isTanslatesLoaded = true;
            break;
        }
    }
    if (!isTanslatesLoaded)
        qCWarning(LogSystem, "No suitable translation found. Falling back to defult (English)");
    // MainWindow w;  // создание экземпляра окна
    // w.show(); // отображение окна
    return 0;
}
