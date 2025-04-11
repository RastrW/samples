#include <QDebug>
#include "test-plugin.hpp"

void TestPlugin::doSomething() const{
    qDebug()<< "hot dog!";
}
