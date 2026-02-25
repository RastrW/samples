#pragma once
#include <QWidget>

class SettingsStackedItemWidget : public QWidget{
    Q_OBJECT
public:
    SettingsStackedItemWidget(QWidget *parent = nullptr);

    virtual void applyChanges() = 0;
signals:
    /// Сигнал об изменении настроек
    void settingsChanged();
};
