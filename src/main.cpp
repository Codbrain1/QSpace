#include <QApplication>  //заголовок включающий основной графический класс приложения
#include <QLocale>       //заголовок включающий класс конвертации чисел и их строковых предсталений на различные языки
#include <QTranslator>   //заголовок класса для перевода на различные языки

#include "mainwindow.h"  //заголовок класса отображаемого окна

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);  // создание экземпляра приложения

    QTranslator translator;                                           // создание экземпляра переводчика на разные языки
    const QStringList uiLanguages = QLocale::system().uiLanguages();  // вытаскиваем доступные в системе языки
    for (const QString &locale : uiLanguages) {
        const QString baseName = "testProject_" + QLocale(locale).name();  // задаем имя приложения в состветствии с
                                                                           // локализацией
        if (translator.load(":/i18n/" + baseName)) {                       // загрузка переводов
            a.installTranslator(&translator);                              // установка переводчика
            break;
        }
    }
    MainWindow w;  // создание экземпляра окна
    w.show();      // отображение окна
    return a.exec();
}
