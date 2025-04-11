#ifndef TEST_PLUGIN_HPP
#define TEST_PLUGIN_HPP
#pragma once
#include <QObject>
#include <QtPlugin>
#include "test-plugin-interface.hpp"

class TestPlugin : public QObject, public TestPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID TestPluginInterface_iid)
    Q_INTERFACES(TestPluginInterface)

public:
    ~TestPlugin() override {}
    void doSomething() const override;
};
#endif // TEST_PLUGIN_HPP
