//#include <iostream>

//using namespace std;

//int main()
//{
//    cout << "Hello World!" << endl;
//    return 0;
//}

#include <assert.h>
#include <QDebug>
#include <QPluginLoader>
#include "test-plugin.hpp"

constexpr auto ABSOLUTE_PATH_TO_PLUGIN =
    "/home/ustas/projects/git_web/samples/qtplugin/build-qtpluginex-Desktop-Debug/libtest-plugin.so";
    //"/path/to/build/libtest-plugin.so";

int main(int argc, char* argv[])
{
    assert(QLibrary::isLibrary(ABSOLUTE_PATH_TO_PLUGIN));
    QPluginLoader loader(ABSOLUTE_PATH_TO_PLUGIN);
    if (auto instance = loader.instance()) {
        if (auto plugin = qobject_cast< TestPluginInterface* >(instance)){
            plugin->doSomething();
        }
        else {
            qDebug()<< "qobject_cast<> returned nullptr";
        }
    }
    else {
      qDebug()<< loader.errorString();
    }
}
