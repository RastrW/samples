#include "mainwindow.h"
#include "app.h"

int main(int argc, char *argv[]){
    App a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
