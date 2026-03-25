#include <QLocale>
#include <QTranslator>
#include <QDebug>
#include <QWebEngineSettings>
#include <QTextCodec>
#include "mainwindow.h"
#include "app.h"

int main(int argc, char *argv[]){

    // отключить sandbox ДО создания QApplication
    qputenv("QTWEBENGINE_DISABLE_SANDBOX", "1");

    // устанавливаем ПЕРЕД созданием QApplication
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif

    App app(argc, argv);

    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251"));
    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("CP866"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    //SetConsoleOutputCP(1251);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    //Compile test
    //pqxx::connection cx{"postgresql://accounting@localhost/company"};
    //pqxx::work tx{cx};

#if(defined(_MSC_VER))
    SetConsoleOutputCP(CP_UTF8);
#endif
    app.setOrganizationName("STC UE");
    app.setApplicationName("QRastr");

    if(!app.init()){
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



    MainWindow w;
    // Qt-синк уже создан в конструкторе MainWindow.
    // Воспроизводим ранние сообщения в Qt-виджет ПРЯМО СЕЙЧАС,
    // пока start() ещё не добавил новых записей.
    app.flushLogCache(w.getProtocolLogSink());

    if (!app.start()) {
        return 200;
    }

    w.initialize(
        app.getQAstraPtr(),
        app.getQTIPtr(),
        app.getQBarsMDPPtr(),
        app.getForms()
        );

    QObject::connect(&app, &QApplication::aboutToQuit, []{
        qInfo() << "ABOUT TO QUIT";
    });

    w.show();
    return app.exec();
}
