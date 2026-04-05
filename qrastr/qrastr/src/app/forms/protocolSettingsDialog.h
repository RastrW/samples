#pragma once
#include <QDialog>

class QCheckBox;
class QDialogButtonBox;

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
};