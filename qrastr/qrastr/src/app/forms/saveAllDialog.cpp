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
    : QDialog(parent), m_pqastra(pqastra), m_FilesLoad(_mFilesLoad)
{
    setWindowTitle(tr("Сохранение файлов"));
    setWindowModality(Qt::ApplicationModal);
    setSizeGripEnabled(true);
    setModal(true);
    resize(800, 300);

    m_cbSelectAll = new QCheckBox(tr("Отметить всё"));
    m_cbSelectAll->setTristate(true);

    m_twSaveFiles = new QTableWidget();
    m_twSaveFiles->setColumnCount(4);
    m_twSaveFiles->setSelectionMode(QAbstractItemView::SingleSelection);
    m_twSaveFiles->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_twSaveFiles->horizontalHeader()->setStretchLastSection(true);
    m_twSaveFiles->verticalHeader()->setVisible(false);
    m_twSaveFiles->setSortingEnabled(true);
    m_twSaveFiles->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QStringList headers;
    headers << "Сохранить" << "Шаблон" << "Файл" << "Путь";
    m_twSaveFiles->setHorizontalHeaderLabels(headers);
    m_twSaveFiles->setColumnWidth(static_cast<int>(_cols::save),  80);
    m_twSaveFiles->setColumnWidth(static_cast<int>(_cols::templ), 120);
    m_twSaveFiles->setColumnWidth(static_cast<int>(_cols::file),  120);
    m_twSaveFiles->setColumnWidth(static_cast<int>(_cols::path),  400);

    connect(m_cbSelectAll, &QCheckBox::clicked, this, [this]() {
        // clicked срабатывает ПОСЛЕ смены состояния Qt'ом.
        // Нас не устраивает автоматический переход PartiallyChecked→Unchecked,
        // поэтому читаем текущее состояние и корректируем его вручную.
        const Qt::CheckState currentState = m_cbSelectAll->checkState();
        selectAllToggled(currentState);
    });
    connect(m_twSaveFiles, &QTableWidget::itemChanged,
            this, &SaveAllDialog::slot_itemChanged);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &SaveAllDialog::slot_buttonBoxAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_cbSelectAll);
    mainLayout->addWidget(m_twSaveFiles);
    mainLayout->addWidget(buttonBox);
}

void SaveAllDialog::selectAllToggled(Qt::CheckState currentState)
{
    Qt::CheckState newState;
    if (currentState == Qt::Unchecked) {
        // Был Checked → пользователь снял → снимаем все
        newState = Qt::Unchecked;
    } else {
        // Был Unchecked или PartiallyChecked → выбираем все
        newState = Qt::Checked;
    }

    // Явно фиксируем состояние cbSelectAll (важно для случая PartiallyChecked)
    m_bUpdating = true;
    m_cbSelectAll->setCheckState(newState);
    for (int row = 0; row < m_twSaveFiles->rowCount(); ++row) {
        QTableWidgetItem* item = m_twSaveFiles->item(row, static_cast<int>(_cols::save));
        if (item)
            item->setCheckState(newState);
    }
    m_bUpdating = false;
}

void SaveAllDialog::slot_itemChanged(QTableWidgetItem* item)
{
    if (m_bUpdating)
        return;
    if (item->column() != static_cast<int>(_cols::save))
        return;

    int nChecked = 0;
    int nTotal   = m_twSaveFiles->rowCount();
    for (int row = 0; row < nTotal; ++row) {
        const QTableWidgetItem* cb = m_twSaveFiles->item(row, static_cast<int>(_cols::save));
        if (cb && cb->checkState() == Qt::Checked)
            ++nChecked;
    }

    m_bUpdating = true;
    if (nChecked == 0)
        m_cbSelectAll->setCheckState(Qt::Unchecked);
    else if (nChecked == nTotal)
        m_cbSelectAll->setCheckState(Qt::Checked);
    else
        m_cbSelectAll->setCheckState(Qt::PartiallyChecked);
    m_bUpdating = false;
}

void SaveAllDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    m_twSaveFiles->setRowCount(0);
    int n_row_num = 0;

    for (auto it = m_FilesLoad.begin(); it != m_FilesLoad.end(); ++it) {
        const QString& _shabl = it.key();
        const QString& _file  = it.value();

        m_dirShabl = QFileInfo(_shabl).path();
        m_twSaveFiles->insertRow(n_row_num);

        QTableWidgetItem* ptwi_checkbox = new QTableWidgetItem();
        ptwi_checkbox->setCheckState(Qt::Checked);

        m_twSaveFiles->setItem(n_row_num, static_cast<int>(_cols::save), ptwi_checkbox);
        m_twSaveFiles->setItem(n_row_num, static_cast<int>(_cols::templ),
                               new QTableWidgetItem(QFileInfo(_shabl).fileName()));
        m_twSaveFiles->setItem(n_row_num, static_cast<int>(_cols::file),
                               new QTableWidgetItem(QFileInfo(_file).fileName()));
        m_twSaveFiles->setItem(n_row_num, static_cast<int>(_cols::path),
                               new QTableWidgetItem(QFileInfo(_file).path()));
        n_row_num++;
    }
}

void SaveAllDialog::slot_buttonBoxAccepted()
{
    for (int n_rownum = 0; n_rownum < m_twSaveFiles->rowCount(); n_rownum++) {
        const QTableWidgetItem* ptwi_checkbox =
            m_twSaveFiles->item(n_rownum, static_cast<int>(_cols::save));
        if (Qt::Checked == ptwi_checkbox->checkState()) {
            const QTableWidgetItem* ptwi_shabl =
                m_twSaveFiles->item(n_rownum, static_cast<int>(_cols::templ));
            const QTableWidgetItem* ptwi_file  =
                m_twSaveFiles->item(n_rownum, static_cast<int>(_cols::file));
            const QTableWidgetItem* ptwi_path  =
                m_twSaveFiles->item(n_rownum, static_cast<int>(_cols::path));

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

