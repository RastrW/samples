#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "astra_exp.h"

int main(int argc, char *argv[])
{
    //nRes = test();
    _idRastr id_rastr = RastrCreate();

    long nRes = 0;
    nRes = Load(id_rastr, R"(/home/ustas/projects/test-rastr/cx195.rg2)", "");
    nRes = Rgm(id_rastr,"");

    return 13;

    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qrastr_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
