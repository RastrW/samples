#include <QLocale>
#include <QTranslator>
#include <QDebug>
#include "mainwindow.h"
#include "app.h"
#include "formsettings.h"

int main(int argc, char *argv[]){
    system("chcp 65001");

    long n_res = 0;

    // устанавливаем ПЕРЕД созданием QApplication
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif

    App app(argc, argv);
    //Compile test
    //pqxx::connection cx{"postgresql://accounting@localhost/company"};
    //pqxx::work tx{cx};

#if(defined(_MSC_VER))
    SetConsoleOutputCP(CP_UTF8);
#endif
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
    w.setForms(app.getForms());

    w.show();

    return app.exec();
}
