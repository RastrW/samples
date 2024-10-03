#include <QLocale>
#include <QTranslator>
#include <QDebug>
#include "mainwindow.h"
#include "app.h"
#include "formsettings.h"


#include <singleton_dclp.hpp>

class Foo
    : public SingletonDclp<Foo>{
public:
    Foo(int n):n_{n}{
    }
    virtual ~Foo(){
        //assert(0);
    }
    void Bar() {
        //assert(0);
    }
private:
    int n_;
};
int main123(int argc, char *argv[]){
    int n_res = 0;
    //QApplication app(argc, argv);
    App app(argc, argv);
    n_res = app.init(); assert(n_res>0);
    app.setWindowIcon(QIcon(":/images/new.png"));

    Foo::Construct(17);
    const Foo* pf = Foo::GetInstance();
    Foo::GetInstance()->Bar();
    Foo::Destruct();

    n_res = app.start();
    if(n_res<0){
        return 200;
    }

    FormSettings fs;
    n_res = fs.init();
    fs.show();
    return app.exec();
}
int main(int argc, char *argv[]){
    long n_res = 0;
    App app(argc, argv);

    n_res = app.init();
    if(n_res<0){
        return 100;
    }
    app.setWindowIcon(QIcon(":/images/new.png"));
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
