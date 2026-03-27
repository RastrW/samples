#include "protocolsettingsdialog.h"
#include <QVBoxLayout>

ProtocolSettingsDialog::ProtocolSettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Свойства протокола"));
    setModal(true);

    m_cbCollapse = new QCheckBox(tr("Сворачивать стадии без ошибок"), this);
    m_cbXml      = new QCheckBox(tr("Копировать буфер в XML"),        this);

    m_buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* lay = new QVBoxLayout(this);
    lay->addWidget(m_cbCollapse);
    lay->addWidget(m_cbXml);
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
    return m_cbCollapse->isChecked();
}
bool ProtocolSettingsDialog::copyAsXml() const {
    return m_cbXml->isChecked();
}

void ProtocolSettingsDialog::load() {
    QSettings s;
    m_cbCollapse->setChecked(s.value(kKeyCollapse, true).toBool());
    m_cbXml     ->setChecked(s.value(kKeyXml,      false).toBool());
}

void ProtocolSettingsDialog::save() {
    QSettings s;
    s.setValue(kKeyCollapse, m_cbCollapse->isChecked());
    s.setValue(kKeyXml,      m_cbXml->isChecked());
}