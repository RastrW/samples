#include <QLocale>
#include <QTranslator>
#include <QDebug>
#include "mainwindow.h"
#include "app.h"

int main(int argc, char *argv[]){
    long n_res = 0;
    App app(argc, argv);
    n_res = app.init();
    if(n_res<0){
        return 100;
    }
/*
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qrastr_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
*/
    MainWindow w;
    n_res = app.start();
    if(n_res<0){
        return 200;
    }
    w.setForms(app.GetForms());
    w.setQAstra(app.getQAstraPtr());
    w.resize(800,500);
    w.show();
    //w.windowHandle()->setScreen(qApp->screens().last());
    //w.showNormal();
    return app.exec();
}
