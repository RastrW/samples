#pragma once
#include <QObject>
#include <string>

// Сигнальный интерфейс — только для подписки.
class ITableEvents : public QObject {
    Q_OBJECT
public:
    explicit ITableEvents(QObject* parent = nullptr)
        : QObject(parent) {}
    virtual ~ITableEvents() = default;

signals:
    ///< изменены данные в диапазоне
    void sig_dataChanged      (const std::string& tname,
                               int row_from, int col_from,
                               int row_to,   int col_to);
    ///< перестроение модели
    void sig_BeginResetModel  (const std::string& tname);
    void sig_EndResetModel    (const std::string& tname);
    ///< вставка строки
    void sig_BeginInsertRow   (const std::string& tname, int first, int last);
    void sig_EndInsertRow     (const std::string& tname);
    ///< удаление строк
    void sig_BeginRemoveRows  (const std::string& tname, int first, int last);
    void sig_EndRemoveRows    (const std::string& tname);
    ///< обновление представлений
    void sig_UpdateModel      (const std::string& tname);
    void sig_UpdateView       (const std::string& tname);
    void sig_ResetModel       (const std::string& tname);
    /// Строки таблицы tname были добавлены/удалены —
    /// все NAMEREF/SUPERENUM, ссылающиеся на неё, должны обновить кеш.
    void sig_ReferenceChanged (const std::string& tname);
};