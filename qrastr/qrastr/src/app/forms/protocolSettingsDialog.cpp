#include "protocolsettingsdialog.h"
#include <QVBoxLayout>

ProtocolSettingsDialog::ProtocolSettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Свойства протокола"));
    setModal(true);

    m_chkCollapse = new QCheckBox(tr("Сворачивать стадии без ошибок"), this);
    m_chkXml      = new QCheckBox(tr("Копировать буфер в XML"),        this);

    m_buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* lay = new QVBoxLayout(this);
    lay->addWidget(m_chkCollapse);
    lay->addWidget(m_chkXml);
    lay->addStretch();
    lay->addWidget(m_buttons);

    load();

    connect(m_buttons, &QDialogButtonBox::accepted, this, [this]{
        save();
        accept();
    });
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

bool ProtocolSettingsDialog::collapseCleanStages() const {
    return m_chkCollapse->isChecked();
}
bool ProtocolSettingsDialog::copyAsXml() const {
    return m_chkXml->isChecked();
}

void ProtocolSettingsDialog::load() {
    QSettings s;
    m_chkCollapse->setChecked(s.value(kKeyCollapse, true).toBool());
    m_chkXml     ->setChecked(s.value(kKeyXml,      false).toBool());
}

void ProtocolSettingsDialog::save() {
    QSettings s;
    s.setValue(kKeyCollapse, m_chkCollapse->isChecked());
    s.setValue(kKeyXml,      m_chkXml->isChecked());
}