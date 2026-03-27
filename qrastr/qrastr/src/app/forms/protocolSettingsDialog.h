#pragma once
#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QSettings>

class ProtocolSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProtocolSettingsDialog(QWidget* parent = nullptr);

    bool collapseCleanStages() const;
    bool copyAsXml()           const;

private:
    void load();
    void save();

    QCheckBox*        m_cbCollapse = nullptr;
    QCheckBox*        m_cbXml      = nullptr;
    QDialogButtonBox* m_buttons    = nullptr;

    static constexpr auto kKeyCollapse = "protocol/collapseCleanStages";
    static constexpr auto kKeyXml      = "protocol/copyAsXml";
};