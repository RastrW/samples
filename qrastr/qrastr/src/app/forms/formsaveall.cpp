#include "formsaveall.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QTableWidget>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include "qastra.h"

formsaveall::formsaveall(QAstra* pqastra, QMap<QString,QString> _mFilesLoad, QWidget *parent)
    : QDialog(parent), m_pqastra(pqastra), m_FilesLoad(_mFilesLoad)
{
    setWindowTitle(tr("Сохранение файлов"));
    setWindowModality(Qt::ApplicationModal);
    setSizeGripEnabled(true);
    setModal(true);
    resize(800, 300);

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

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &formsaveall::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_twSaveFiles);
    mainLayout->addWidget(buttonBox);
}

void formsaveall::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    m_twSaveFiles->setRowCount(0);
    int n_row_num = 0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    for (auto [_shabl, _file] : asKeyValueRange(mFilesLoad)) {
#else
    for (auto [_shabl, _file] : m_FilesLoad.asKeyValueRange()) {
#endif
        m_dirShabl = QFileInfo(_shabl).path(); // assume all templates at one directory
        m_twSaveFiles->insertRow(n_row_num);

        QTableWidgetItem* ptwi_checkbox = new QTableWidgetItem();
        ptwi_checkbox->setCheckState(Qt::Checked);
        ptwi_checkbox->data(Qt::CheckStateRole);
        m_twSaveFiles->setItem(n_row_num, static_cast<int>(_cols::save),
                               ptwi_checkbox);
        m_twSaveFiles->setItem(n_row_num, static_cast<int>(_cols::templ),
                               new QTableWidgetItem(QFileInfo(_shabl).fileName()));
        m_twSaveFiles->setItem(n_row_num, static_cast<int>(_cols::file),
                               new QTableWidgetItem(QFileInfo(_file).fileName()));
        m_twSaveFiles->setItem(n_row_num, static_cast<int>(_cols::path),
                               new QTableWidgetItem(QFileInfo(_file).path()));
        n_row_num++;
    }
}

void formsaveall::on_buttonBox_accepted()
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

