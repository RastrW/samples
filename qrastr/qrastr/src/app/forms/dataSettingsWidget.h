#pragma once
#include "settingsStackedItemWidget.h"

class QGroupBox;
class QSpinBox;
class QPushButton;
class QCheckBox;
class QLineEdit;

class DataSettingsWidget : public SettingsStackedItemWidget {
    Q_OBJECT

public:
    explicit DataSettingsWidget(QWidget *parent = nullptr);
    ~DataSettingsWidget() = default;

    void applyChanges() override;
private slots:
    /// Восстановить директорию пользователя по умолчанию
    void onRestoreUserDirectoryClicked();
    /// Обработчик выбора пути к XML протоколу
    void onSelectXmlProtocolPath();
private:
    /// Создать UI элементы
    void setupUI();

    // UI элементы
    QGroupBox*  pGroupBoxMaintenance_ {nullptr};
    QSpinBox*   pSpinBoxNumItems_ {nullptr};
    QPushButton* pPushButtonRestore_ {nullptr};

    QGroupBox*  pGroupBoxHistory_ {nullptr};
    QCheckBox*  pCheckBoxRememberHistory_ {nullptr};
    QPushButton* pPushButtonSelectXmlPath_ {nullptr};
    QLineEdit*  pLineEditXmlPath_ {nullptr};

    int m_maxRecent;  // Временное хранилище
    bool m_hasChanges {false};
};
