#include "saveAllDialog.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QTableWidget>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include "qastra.h"

SaveAllDialog::SaveAllDialog(QAstra* pqastra, QMap<QString,QString> _mFilesLoad, QWidget *parent)
    : CheckboxListDialog(static_cast<int>(_cols::save), parent),
    m_pqastra(pqastra),
    m_FilesLoad(_mFilesLoad)
{
    setWindowTitle(tr("Сохранение файлов"));
    setWindowModality(Qt::ApplicationModal);
    setSizeGripEnabled(true);
    setModal(true);
    resize(800, 300);

    m_twList->setColumnCount(4);
    m_twList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_twList->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_twList->horizontalHeader()->setStretchLastSection(true);
    m_twList->verticalHeader()->setVisible(false);
    m_twList->setSortingEnabled(true);
    m_twList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QStringList headers;
    headers << "Сохранить" << "Шаблон" << "Файл" << "Путь";
    m_twList->setHorizontalHeaderLabels(headers);
    m_twList->setColumnWidth(static_cast<int>(_cols::save),  80);
    m_twList->setColumnWidth(static_cast<int>(_cols::templ), 120);
    m_twList->setColumnWidth(static_cast<int>(_cols::file),  120);
    m_twList->setColumnWidth(static_cast<int>(_cols::path),  400);

    initCheckboxControls(tr("Отметить всё")); // подключает сигналы

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &SaveAllDialog::slot_buttonBoxAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_cbSelectAll);
    mainLayout->addWidget(m_twList);
    mainLayout->addWidget(buttonBox);
}

void SaveAllDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    m_twList->setRowCount(0);
    int n_row_num = 0;

    for (auto it = m_FilesLoad.begin(); it != m_FilesLoad.end(); ++it) {
        const QString& _shabl = it.key();
        const QString& _file  = it.value();

        m_dirShabl = QFileInfo(_shabl).path();
        m_twList->insertRow(n_row_num);

        QTableWidgetItem* ptwi_checkbox = new QTableWidgetItem();
        ptwi_checkbox->setCheckState(Qt::Checked);

        m_twList->setItem(n_row_num, static_cast<int>(_cols::save), ptwi_checkbox);
        m_twList->setItem(n_row_num, static_cast<int>(_cols::templ),
                               new QTableWidgetItem(QFileInfo(_shabl).fileName()));
        m_twList->setItem(n_row_num, static_cast<int>(_cols::file),
                               new QTableWidgetItem(QFileInfo(_file).fileName()));
        m_twList->setItem(n_row_num, static_cast<int>(_cols::path),
                               new QTableWidgetItem(QFileInfo(_file).path()));
        n_row_num++;
    }
}

void SaveAllDialog::slot_buttonBoxAccepted()
{
    for (int n_rownum = 0; n_rownum < m_twList->rowCount(); n_rownum++) {
        const QTableWidgetItem* ptwi_checkbox =
            m_twList->item(n_rownum, static_cast<int>(_cols::save));
        if (Qt::Checked == ptwi_checkbox->checkState()) {
            const QTableWidgetItem* ptwi_shabl =
                m_twList->item(n_rownum, static_cast<int>(_cols::templ));
            const QTableWidgetItem* ptwi_file  =
                m_twList->item(n_rownum, static_cast<int>(_cols::file));
            const QTableWidgetItem* ptwi_path  =
                m_twList->item(n_rownum, static_cast<int>(_cols::path));

            QFileInfo qfileinfo;
            qfileinfo.setFile(QDir(ptwi_path->text()), ptwi_file->text());

            QFileInfo qshablinfo;
            qshablinfo.setFile(m_dirShabl, ptwi_shabl->text());

            std::string sfile  = qfileinfo.absoluteFilePath().toStdString().c_str();
            std::string sshabl = qshablinfo.isFile()
                                     ? qshablinfo.absoluteFilePath().toStdString().c_str()
                                     : "";

            m_pqastra->Save(sfile, sshabl);
        }
    }
}

