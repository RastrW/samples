#include <QLocale>
#include <QTranslator>
#include <QDebug>
#include "mainwindow.h"
#include "app.h"
#include "formsettings.h"
//#include <SDL3\SDL.h>
//#include <pqxx/pqxx>
/*
int main23(int argc, char *argv[]){
    int n_res = 0;
    //QApplication app(argc, argv);
    App app(argc, argv);
    n_res = app.init(); assert(n_res>0);
    app.setWindowIcon(QIcon(":/images/rastr.png"));

    n_res = app.start();
    if(n_res<0){
        return 200;
    }

    FormSettings fs;
    n_res = fs.init(app.getQAstraPtr());
    fs.show();
    return app.exec();
}
*/
int main(int argc, char *argv[]){
    long n_res = 0;

    // устанавливаем ПЕРЕД созданием QApplication
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif

    App app(argc, argv);

    //Compile test
    //pqxx::connection cx{"postgresql://accounting@localhost/company"};
    //pqxx::work tx{cx};

    app.setOrganizationName("Trolltech");
    app.setApplicationName("QRastr");

    n_res = app.init();
    if(n_res<0){
        return 100;
    }
    app.setWindowIcon(QIcon(":/images/rastr.png"));
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
   // SDL_Init(SDL_INIT_VIDEO); // Basics of SDL, init what you need to use
    MainWindow w;

    n_res = app.start();
    if(n_res<0){
        return 200;
    }
    w.setQAstra(app.getQAstraPtr());
    w.setQTI(app.getQTIPtr());
    w.setQBarsMDP(app.getQBarsMDPPtr());
    w.setForms(app.GetForms());
    //w.resize(1200,800);
    w.show();
    //w.windowHandle()->setScreen(qApp->screens().last());
    //w.showNormal();
    return app.exec();
}
