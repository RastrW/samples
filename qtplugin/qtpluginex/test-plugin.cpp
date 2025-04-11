#include <QDebug>
#include "test-plugin.h"

void TestPlugin::doSomething() const{
    qDebug()<< "hot dog!";
}
