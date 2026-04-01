#include <QLocale>
#include <QTranslator>
#include <QDebug>
#include <QWebEngineSettings>
#include <QTextCodec>
#include <QSplashScreen>
#include <QProgressBar>
#include "mainwindow.h"
#include "app.h"

void updateSplash(int pct, const QString& msg,
                  QSplashScreen* splash, QProgressBar* pb) {

    pb->setValue(pct);
    // Текст — поверх изображения, снизу по центру
    splash->showMessage(
        msg, Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
    QApplication::processEvents();
};

int main(int argc, char *argv[]) {
    // отключить sandbox ДО создания QApplication
    qputenv("QTWEBENGINE_DISABLE_SANDBOX", "1");
    // устанавливаем ПЕРЕД созданием QApplication
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif
    App app(argc, argv);
#if defined(_MSC_VER)
    SetConsoleOutputCP(CP_UTF8);
#endif
    app.setOrganizationName("STC UE");
    app.setApplicationName("QRastr");
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
    // ── Сплэш-экран ──────────────────────────────────────────────
    QPixmap splashPix(":/images/new_style/abstract.png");
    QPixmap scaled = splashPix.scaled(
        1000, 800,                 // нужный размер
        Qt::KeepAspectRatio,      // сохраняем пропорции
        Qt::SmoothTransformation // качественное масштабирование
        );

    auto* splash = new QSplashScreen(scaled,
                                     Qt::WindowStaysOnTopHint);

    // Прогресс-бар поверх изображения
    auto* pb = new QProgressBar(splash);
    pb->setGeometry(10, scaled.height() - 30,
                    scaled.width() - 20, 10);
    pb->setRange(0, 100);
    pb->setValue(0);
    pb->setTextVisible(false);
    // Минимальный стиль, чтобы не зависеть от темы
    pb->setStyleSheet(
        "QProgressBar { background: #1a2a3a; border: 1px solid #3a5a7a; }"
        "QProgressBar::chunk { background: #4a9adf; }");

    splash->show();
    QApplication::processEvents();

    auto updateSplash = [&](int pct, const QString& msg) {
        pb->setValue(pct);
        // Текст — поверх изображения, снизу по центру
        splash->showMessage(
            msg, Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
        QApplication::processEvents();
    };

    updateSplash(5, QObject::tr("Инициализация..."));
    if (!app.init()) {
        splash->close();
        delete splash;
        return 100;
    }
    updateSplash(25, QObject::tr("Настройки загружены"));
    MainWindow w;
    // Qt-синк уже создан в конструкторе MainWindow.
    // Воспроизводим ранние сообщения в Qt-виджет ПРЯМО СЕЙЧАС,
    // пока start() ещё не добавил новых записей.
    app.flushLogCache(w.getProtocolLogSink());
    updateSplash(30, QObject::tr("Загрузка плагинов..."));

    // Подключаем сигналы прогресса из App::start()
    QObject::connect(&app, &App::sig_progressChanged,
                     [&](int pct, const QString& msg) {
                         updateSplash(pct, msg);
                     });

    if (!app.start()) {
        splash->close();
        delete splash;
        return 200;
    }
    updateSplash(85, QObject::tr("Инициализация интерфейса..."));

    w.initialize(
        app.getQAstraPtr(),
        app.getQTIPtr(),
        app.getQBarsMDPPtr(),
        app.getForms());

    updateSplash(100, QObject::tr("Готово"));

    QObject::connect(&app, &QApplication::aboutToQuit, [] {
        qInfo() << "ABOUT TO QUIT";
    });

    w.show();
    splash->finish(&w);   // закрыть сплэш после показа MainWindow
    delete splash;

    return app.exec();
}
