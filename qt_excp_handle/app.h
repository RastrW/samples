#ifndef APP_H
#define APP_H

#include <QObject>
#include <QApplication>
#include <QDebug>

class App Q_DECL_FINAL
    : public QApplication{
    Q_OBJECT
public:
    App(int &argc, char **argv)
        :  QApplication(argc, argv) {
    }
    ~App() override {
        printf("exit()");
    }
    //! Watch for QFileOpenEvent.
    bool event( QEvent *event ) override {
        const bool done = QApplication::event( event);
        //assert(done==false);
        return done;
    }
    bool notify(QObject* receiver, QEvent* event) override {
        bool done = false;
        try {
            done = QApplication::notify(receiver, event);
        }catch(std::exception& ex){
            printf("excep [%s]\n", ex.what());
        }catch (...){
            printf("excep UNKNOWN\n");
        }
        return done;
     }
};

#endif // APP_H
