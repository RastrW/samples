#pragma once

#include <QDialog>
#include <vector>

#include "Palette.h"

namespace Ui {
    class CondFormatManager;
}

class CondFormat;
class QTreeWidgetItem;
class QAbstractButton;
class QTreeWidgetItem;
class QLabel;
class QTreeWidget;
class QDialogButtonBox;

///@class QDialog, получает vector<CondFormat>, позволяет пользователю редактировать,
/// возвращает результат через getCondFormats(). Ничего не знает о модели или файлах.
class CondFormatManager : public QDialog
{
    Q_OBJECT

public:
    explicit CondFormatManager(const std::vector<CondFormat>& condFormats, const QString& encoding, QWidget *parent = nullptr);
    ~CondFormatManager() override;

    std::vector<CondFormat> getCondFormats() const;

public slots:
    void itemClicked(QTreeWidgetItem* item, int column);

private slots:
    void addNewItem();
    void addItem(const CondFormat& aCondFormat);
    void removeItem();
    void moveItem(int offset);
    void upItem();
    void downItem();
    void buttonBoxClicked(QAbstractButton* button);
private:
    enum Columns {
        ColumnForeground = 0,
        ColumnBackground,
        ColumnFont,
        ColumnSize,
        ColumnBold,
        ColumnItalic,
        ColumnUnderline,
        ColumnAlignment,
        ColumnFilter
    };
    // ── Виджеты ────────────────────────
    QLabel*          m_labelTitle;
    QToolButton*     m_buttonAdd;
    QToolButton*     m_buttonRemove;
    QToolButton*     m_buttonUp;
    QToolButton*     m_buttonDown;
    QTreeWidget*     m_table;
    QDialogButtonBox* m_buttonBox;
    // ── Данные ───────────────────────────────────────────────────────────────
    Palette m_condFormatPalette;
    QString m_encoding;

    void setupWidgets();
};
