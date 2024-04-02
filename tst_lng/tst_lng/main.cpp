#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /*
    QTranslator translator;
    const QStringList uiLanguages1 = QLocale::system().uiLanguages();
    const QStringList uiLanguages {"ru-RU","ru"};
    for (const QString &locale : uiLanguages) {
        const QString baseName = "tst_lng_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }*/
    QTranslator translator;
    bool blRes = translator.load("/home/ustas/projects/git_web/samples/tst_lng/tst_lng/tst_lng_ru_RU.qm");// must be compiled into *.qm    //blRes = translator.load("/home/ustas/projects/git_web/samples/tst_lng/tst_lng/i18n/tst_lng_ru_RU.ts");
    //blRes = translator.load(":/tst_lng_ru_RU");

    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
