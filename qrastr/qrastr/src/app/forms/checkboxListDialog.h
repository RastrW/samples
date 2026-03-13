#pragma once
#include <QDialog>

class QCheckBox;
class QTableWidget;

/// Базовый класс для диалогов с таблицей, первая колонка которой —
/// чекбоксы, и кнопкой «Отметить всё» над ней.
class CheckboxListDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CheckboxListDialog(int checkboxColumn = 0, QWidget* parent = nullptr);

protected:
    /// Инициализирует виджеты и соединяет сигналы.
    /// Вызывать из конструктора наследника ПОСЛЕ того, как таблица
    /// полностью настроена (количество колонок, заголовки и т.д.).
    void initCheckboxControls(const QString& selectAllLabel = tr("Отметить всё"));

    /// Обновляет состояние cbSelectAll по текущему содержимому таблицы.
    void updateSelectAll();

    QCheckBox*    m_cbSelectAll  = nullptr;
    QTableWidget* m_twList       = nullptr;
    bool          m_bUpdating    = false;   ///< предотвращает рекурсию

private slots:
    void slot_itemChanged(QTableWidgetItem* item);

private:
    void selectAllToggled(Qt::CheckState currentState);

    int m_checkboxColumn;
};
