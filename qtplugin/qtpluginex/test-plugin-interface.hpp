#ifndef TEST_PLUGIN_INTERFACE
#define TEST_PLUGIN_INTERFACE
#include <QtPlugin>

class TestPluginInterface
{
public:
    virtual ~TestPluginInterface() {}
    virtual void doSomething() const = 0;
};

#define TestPluginInterface_iid "whatever.you.want.TestPluginInterface"
Q_DECLARE_INTERFACE(TestPluginInterface, TestPluginInterface_iid)
#endif // TEST_PLUGIN_INTERFACE
